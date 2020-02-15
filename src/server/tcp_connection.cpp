#include "server/tcp_connection.hpp"

#include <iomanip>
#include <iostream>

// Include the ConnectionManager class explicitly to allow symbol lookup to
// succeed.
#include "server/connection_manager.hpp"

std::shared_ptr<TcpConnection> TcpConnection::Create(
    boost::asio::io_context& io_context, ConnectionManager& manager,
    std::string endpoint_name) {
  return std::shared_ptr<TcpConnection>(new TcpConnection(
        io_context, manager, endpoint_name));
}

TcpConnection::TcpConnection(
    boost::asio::io_context& io_context, ConnectionManager& manager,
    std::string endpoint_name)
    : socket_(io_context),
      strand_(io_context),
      manager_(manager),
      endpoint_name_(endpoint_name) {
  logger_ = spdlog::get("connection");
  logger_->trace("creating connection to {}", endpoint_name);
}

TcpConnection::~TcpConnection() {
  logger_->trace("destructor called for connection to {}", endpoint_name_);
}

void TcpConnection::Start() {
  boost::asio::post(strand_,
      std::bind(&TcpConnection::StartReadHeader, shared_from_this()));;
}

void TcpConnection::Stop() {
  logger_->trace("closing connection to {}", endpoint_name_);
  boost::system::error_code error;
  socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_send, error);
  if (error) {
    logger_->error("socket close error: {}", error.message());
  }
}

void TcpConnection::StartReadHeader() {
  boost::asio::async_read(
      socket_, boost::asio::buffer(inbound_header_, kHeaderSize),
      std::bind(&TcpConnection::HandleReadHeader, shared_from_this(),
                std::placeholders::_1, std::placeholders::_2));
}

void TcpConnection::HandleReadHeader(const boost::system::error_code& error,
                                     std::size_t bytes_transferred) {
  if (!error) {
    std::string header(inbound_header_, kHeaderSize);
    std::istringstream is(header);
    std::size_t data_size = 0;
    if (!(is >> std::hex >> data_size)) {
      logger_->error("failed to parse header in message from {}, header: {}",
          endpoint_name_, header);
      return;
    }

    // Read in the correct number of bytes for the message.
    boost::asio::async_read(
        socket_, input_buffer_,
        boost::asio::transfer_exactly(data_size),
        std::bind(&TcpConnection::HandleReadBody, shared_from_this(),
                  std::placeholders::_1, std::placeholders::_2));
  } else {
    logger_->error("handle read header error: {}", error.message());
    manager_.RemoveConnection(shared_from_this());
    socket_.close();
  }
}

void TcpConnection::HandleReadBody(const boost::system::error_code& error,
                                   std::size_t bytes_transferred) {
  if (!error) {
    assert(input_buffer_.size() == bytes_transferred);

    // Read the message out of the buffer.
    const char* buf =
        boost::asio::buffer_cast<const char*>(input_buffer_.data());
    const std::string data(buf, bytes_transferred);
    input_buffer_.consume(bytes_transferred);

    Message message;
    message.ParseFromString(data);

    if (message.type() == Message_MessageType_UNKNOWN) {
      logger_->error("parsed unknown message of size {} with body: {}",
          bytes_transferred, data);
    }

    manager_.Handle(message, shared_from_this());

    // Setup messages cause the server to add teh connection to the set of
    // tracked connections, which will call Start() on the connection again.
    if (message.type() != Message_MessageType_SETUP) {
      boost::asio::post(strand_,
          std::bind(&TcpConnection::StartReadHeader, shared_from_this()));;
    }
  } else {
    logger_->error("handle read body error: {}", error.message());
    manager_.RemoveConnection(shared_from_this());
    socket_.close();
  }
}

void TcpConnection::Write(const Message& message) {
  const std::lock_guard<std::mutex> lock(mutex_);

  if (message.type() == Message_MessageType_UNKNOWN) {
    logger_->error("unknown message type, not writing...");
    return;
  }

  // Write the message size first followed by the message.
  std::ostringstream message_stream;
  std::size_t message_size = message.ByteSizeLong();
  std::ostringstream header_stream;
  header_stream << std::setw(kHeaderSize) << std::hex << message_size;
  if (!header_stream || header_stream.str().size() != kHeaderSize) {
    logger_->error("failed to serialize message size");
    return;
  }
  message_stream << header_stream.str();

  std::string serialized;
  message.SerializeToString(&serialized);
  message_stream << serialized;

  assert(serialized.size() == message_size);
  assert(message_stream.str().size() ==
      header_stream.str().size() + serialized.size());

  out_queue_.push_back(message_stream.str());

  if (out_queue_.size() > 1) {
    return;
  }

  boost::asio::post(strand_, std::bind(
      static_cast<void (TcpConnection::*)(void)>(&TcpConnection::Write),
      shared_from_this()));
}


void TcpConnection::Write() {
  const std::lock_guard<std::mutex> lock(mutex_);

  if (out_queue_.empty()) {
    logger_->warn("out queue empty on attempted write to {}", endpoint_name_);
    return;
  }

  const std::string& message = out_queue_.front();
  boost::asio::async_write(socket_, boost::asio::buffer(message),
                           std::bind(&TcpConnection::HandleWrite,
                                     shared_from_this(),
                                     std::placeholders::_1,
                                     std::placeholders::_2));
}

void TcpConnection::HandleWrite(const boost::system::error_code& error,
                                size_t bytes_transferred) {
  const std::lock_guard<std::mutex> lock(mutex_);

  if (error) {
    logger_->error("write error: {}", error.message());
    return;
  }

  assert(out_queue_.size() >= 1);
  out_queue_.pop_front();

  if (!out_queue_.empty()) {
    // Send next message in queue.
    boost::asio::post(strand_, std::bind(
        static_cast<void (TcpConnection::*)(void)>(&TcpConnection::Write),
        shared_from_this()));
  }
}
