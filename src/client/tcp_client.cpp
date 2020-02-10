#include "client/tcp_client.hpp"

#include "google/protobuf/util/delimited_message_util.h"

TcpClient::TcpClient(
    boost::asio::io_context& io_context, std::string& name,
    std::unordered_map<std::string, std::deque<Command>>& workload)
    : socket_(io_context),
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

    StartRead();

    for (const auto& [client, _] : workload_) {
      SendNextRequest(client);
    }
  }
}

void TcpClient::StartRead() {
  boost::asio::async_read(socket_, input_buffer_,
                          boost::asio::transfer_at_least(1),
                          std::bind(&TcpClient::HandleRead, this,
                                    std::placeholders::_1,
                                    std::placeholders::_2));
}

void TcpClient::HandleRead(const boost::system::error_code& error,
                           std::size_t bytes_transferred) {
  if (!error) {
    std::istream reader(&input_buffer_);
    ::google::protobuf::io::IstreamInputStream raw_istream(&reader);

    Message message;
    bool clean_eof;
    ::google::protobuf::util::ParseDelimitedFromZeroCopyStream(&message, &raw_istream, &clean_eof);

    if (message.type() == Message_MessageType_RESPONSE) {
      Response r;
      message.message().UnpackTo(&r);

      logger_->info("Value ({}:{}): {}", r.client(), r.sequence_number(), r.value());

      SendNextRequest(r.client());
    }

    StartRead();
  } else {
    logger_->error("receive error: {}", error.message());
    Stop();
  }
}

void TcpClient::StartWrite(boost::asio::streambuf& stream_buffer) {
  boost::asio::async_write(
      socket_, stream_buffer.data(),
      std::bind(&TcpClient::HandleWrite, this, std::placeholders::_1));
}

void TcpClient::HandleWrite(const boost::system::error_code& error) {
  if (error) {
    Stop();
  }
}

void TcpClient::SendNextRequest(const std::string& client) {
  boost::asio::streambuf stream_buffer;
  GetNextMessage(stream_buffer, client);
  if (stream_buffer.size() > 0) {
    logger_->trace("writing {} bytes", stream_buffer.size());
    send_time_[client] = std::chrono::steady_clock::now();
    StartWrite(stream_buffer);
  }
}

void TcpClient::GetNextMessage(boost::asio::streambuf& stream_buffer,
                               const std::string& client) {
  auto& workload = workload_.at(client);
  if (workload.empty()) {
    return;
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

  std::ostream ostream(&stream_buffer);
  ::google::protobuf::util::SerializeDelimitedToOstream(m, &ostream);
}
