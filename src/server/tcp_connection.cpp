// Copyright 2019 Lukas Joswiak

#include "server/tcp_connection.hpp"

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
    std::string message(
        boost::asio::buffers_begin(input_buffer_.data()),
        boost::asio::buffers_begin(input_buffer_.data()) + bytes_transferred);
    input_buffer_.consume(bytes_transferred);

    manager_.Handle(message, shared_from_this());

    StartRead();
  } else {
    manager_.Remove(shared_from_this());
  }
}

void TcpConnection::StartWrite(const std::string& message) {
  boost::asio::async_write(socket_, boost::asio::buffer(message),
                           std::bind(&TcpConnection::HandleWrite,
                                     shared_from_this(),
                                     std::placeholders::_1,
                                     std::placeholders::_2));
}

void TcpConnection::HandleWrite(const boost::system::error_code& error,
                                size_t bytes_transferred) {}
