// Copyright 2019 Lukas Joswiak

#include "server/tcp_connection.hpp"

#include <iostream>

std::shared_ptr<TcpConnection> TcpConnection::Create(
    boost::asio::io_context& io_context) {
  return std::shared_ptr<TcpConnection>(new TcpConnection(io_context));
}

TcpConnection::TcpConnection(boost::asio::io_context& io_context)
    : socket_(io_context) { }

void TcpConnection::Start() {
  StartRead();
  StartWrite("hello\n");
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
    std::cout << "Read line: " << input_buffer_ << std::endl;
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
