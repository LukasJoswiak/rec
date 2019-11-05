// Copyright 2019 Lukas Joswiak

#include "server/tcp_connection.hpp"

#include <iostream>

// Include the Handler class explicitly to allow symbol lookup to succeed.
#include "server/handler.hpp"

std::shared_ptr<TcpConnection> TcpConnection::Create(
    boost::asio::io_context& io_context, Handler& handler) {
  return std::shared_ptr<TcpConnection>(new TcpConnection(io_context, handler));
}

TcpConnection::TcpConnection(
    boost::asio::io_context& io_context, Handler& handler)
    : socket_(io_context),
      handler_(handler) {}

std::string TcpConnection::LocalEndpoint() {
  std::string address = socket_.local_endpoint().address().to_string();
  auto port = socket_.local_endpoint().port();
  return address + ":" + std::to_string(port);
}

std::string TcpConnection::RemoteEndpoint() {
  std::string address = socket_.remote_endpoint().address().to_string();
  auto port = socket_.remote_endpoint().port();
  return address + ":" + std::to_string(port);
}

void TcpConnection::Start() {
  StartRead();
}

void TcpConnection::StartRead() {
  boost::asio::async_read_until(
      socket_, boost::asio::dynamic_buffer(input_buffer_), '\n',
      std::bind(&TcpConnection::HandleRead, shared_from_this(),
                std::placeholders::_1, std::placeholders::_2));
}

void TcpConnection::HandleRead(const boost::system::error_code& error,
                               std::size_t n) {
  if (!error) {
    handler_.Handle(input_buffer_, shared_from_this());

    input_buffer_.clear();
    StartRead();
  } else {
    std::cerr << "Error on receive: " << error.message() << std::endl;
  }
}

void TcpConnection::StartWrite(const std::string& message) {
  boost::asio::async_write(socket_, boost::asio::buffer(message),
                           std::bind(&TcpConnection::HandleWrite,
                                     shared_from_this(),
                                     std::placeholders::_1,
                                     std::placeholders::_2));
}

void TcpConnection::HandleWrite(const boost::system::error_code& ec,
                                size_t bytes_transferred) {
  std::cout << "Wrote " << bytes_transferred << " bytes" << std::endl;
}
