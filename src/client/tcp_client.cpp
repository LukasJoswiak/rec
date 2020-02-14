#include "client/tcp_client.hpp"

#include <iomanip>

TcpClient::TcpClient(
    boost::asio::io_context& io_context, std::string& name,
    std::unordered_map<std::string, std::deque<Command>>& workload)
    : socket_(io_context),
      strand_(io_context),
      name_(name),
      workload_(workload) {
  logger_ = spdlog::get("client");
}

void TcpClient::Start(boost::asio::ip::tcp::resolver::results_type endpoints) {
  endpoints_ = endpoints;
  StartConnect(endpoints.begin());
}

void TcpClient::Stop() {
  boost::system::error_code error;
  socket_.close(error);
}

void TcpClient::StartConnect(
    boost::asio::ip::tcp::resolver::results_type::iterator endpoint_iter) {
  if (endpoint_iter != endpoints_.end()) {
    socket_.async_connect(endpoint_iter->endpoint(),
                          std::bind(&TcpClient::HandleConnect, this,
                                    std::placeholders::_1, endpoint_iter));
  } else {
    Stop();
  }
}

void TcpClient::HandleConnect(
    const boost::system::error_code& error,
    boost::asio::ip::tcp::resolver::results_type::iterator endpoint_iter) {
  if (!socket_.is_open()) {
    logger_->warn("connect timed out");
    StartConnect(++endpoint_iter);
  } else if (error) {
    logger_->error("connect failed: {}", error.message());
    socket_.close();
    StartConnect(++endpoint_iter);
  } else {
    logger_->info("connected to {}",
        endpoint_iter->endpoint().address().to_string());

    boost::asio::post(strand_,
        std::bind(&TcpClient::StartReadHeader, this));;

    for (const auto& [client, _] : workload_) {
      SendNextRequest(client);
    }
  }
}

void TcpClient::StartReadHeader() {
  boost::asio::async_read(
      socket_, boost::asio::buffer(inbound_header_, kHeaderSize),
      std::bind(&TcpClient::HandleReadHeader, this,
                std::placeholders::_1, std::placeholders::_2));
}

void TcpClient::HandleReadHeader(const boost::system::error_code& error,
                           std::size_t bytes_transferred) {
  if (!error) {
    std::string header(inbound_header_, kHeaderSize);
    std::istringstream is(header);
    std::size_t data_size = 0;
    if (!(is >> std::hex >> data_size)) {
      logger_->error("failed to parse header");
      return;
    }

    // Read in the correct number of bytes for the message.
    boost::asio::async_read(
        socket_, input_buffer_,
        boost::asio::transfer_exactly(data_size),
        std::bind(&TcpClient::HandleReadBody, this,
                  std::placeholders::_1, std::placeholders::_2));
  } else {
    logger_->error("receive error: {}", error.message());
    Stop();
  }
}

void TcpClient::HandleReadBody(const boost::system::error_code& error,
                               std::size_t bytes_transferred) {
  if (!error) {
    assert(input_buffer_.size() == bytes_transferred);

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

    if (message.type() == Message_MessageType_RESPONSE) {
      Response r;
      message.message().UnpackTo(&r);

      auto now = std::chrono::steady_clock::now();
      auto response_time =
          std::chrono::duration_cast<std::chrono::microseconds>(
              now - send_time_[r.client()]);

      logger_->info("Value ({}:{}, {} microseconds): {}", r.client(),
          r.sequence_number(), response_time.count(), r.value());

      SendNextRequest(r.client());
    }

    boost::asio::post(strand_,
        std::bind(&TcpClient::StartReadHeader, this));;
  } else {
    logger_->error("handle read body error: {}", error.message());
  }
}

void TcpClient::StartWrite() {
  assert(out_queue_.size() >= 1);
  const std::string& message = out_queue_.front();
  boost::asio::async_write(
      socket_, boost::asio::buffer(message),
      std::bind(&TcpClient::HandleWrite, this, std::placeholders::_1));
}

void TcpClient::HandleWrite(const boost::system::error_code& error) {
  if (error) {
    logger_->error("handle write error: {}", error.message());
    Stop();
  }

  std::size_t size = out_queue_.size();
  assert(size >= 1);
  out_queue_.pop_front();
  --size;

  if (size > 0) {
    StartWrite();
  }
}

void TcpClient::SendNextRequest(const std::string& client) {
  if (auto serialized = GetNextMessage(client)) {
    // TODO: Don't set start time until message is written.
    send_time_[client] = std::chrono::steady_clock::now();
    out_queue_.push_back(*serialized);

    if (out_queue_.size() > 1) {
      return;
    }

    StartWrite();
  }
}

std::optional<std::string> TcpClient::GetNextMessage(
    const std::string& client) {
  auto& workload = workload_.at(client);
  if (workload.empty()) {
    return std::nullopt;
  }

  Command command = workload.front();
  workload.pop_front();

  Request r;
  r.set_allocated_command(new Command(command));
  r.set_source(name_);

  Message m;
  m.set_type(Message_MessageType_REQUEST);
  m.mutable_message()->PackFrom(r);
  m.set_from(name_);

  // Build serialized message composed of four-byte message length follewed by
  // the serialized message contents.
  std::ostringstream message_stream;
  std::size_t size = m.ByteSizeLong();
  std::ostringstream header_stream;
  // Write header in hex.
  header_stream << std::setw(kHeaderSize) << std::hex << size;
  if (!header_stream || header_stream.str().size() != kHeaderSize) {
    logger_->error("failed to serialize message size");
    return std::nullopt;
  }
  message_stream << header_stream.str();

  // Serialize and append message.
  std::string serialized;
  m.SerializeToString(&serialized);
  assert(serialized.size() == size);
  message_stream << serialized;
  return message_stream.str();
}
