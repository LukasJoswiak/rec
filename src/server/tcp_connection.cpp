#include "server/tcp_connection.hpp"

#include <iostream>

#include "google/protobuf/util/delimited_message_util.h"
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
      manager_(manager),
      endpoint_name_(endpoint_name) {}

TcpConnection::~TcpConnection() {
  boost::system::error_code error;
  socket_.close(error);
}

void TcpConnection::Start() {
  StartRead();
}

void TcpConnection::StartRead() {
  boost::asio::async_read(
      socket_, input_buffer_,
      boost::asio::transfer_at_least(1),
      std::bind(&TcpConnection::HandleRead, shared_from_this(),
                std::placeholders::_1, std::placeholders::_2));
}

void TcpConnection::HandleRead(const boost::system::error_code& error,
                               std::size_t bytes_transferred) {
  if (!error) {
    std::istream istream(&input_buffer_);
    ::google::protobuf::io::IstreamInputStream raw_istream(&istream);

    // Parse the message size followed by the message.
    Message message;
    bool clean_eof;
    ::google::protobuf::util::ParseDelimitedFromZeroCopyStream(
        &message, &raw_istream, &clean_eof);

    manager_.Handle(message, shared_from_this());

    StartRead();
  } else {
    manager_.RemoveConnection(shared_from_this());
  }
}

void TcpConnection::StartWrite(const Message& message) {
  // Write the message size first followed by the message.
  boost::asio::streambuf stream_buffer;
  std::ostream ostream(&stream_buffer);

  ::google::protobuf::util::SerializeDelimitedToOstream(message, &ostream);

  boost::asio::async_write(socket_, stream_buffer.data(),
                           std::bind(&TcpConnection::HandleWrite,
                                     shared_from_this(),
                                     std::placeholders::_1,
                                     std::placeholders::_2));
}

void TcpConnection::HandleWrite(const boost::system::error_code& error,
                                size_t bytes_transferred) {}
