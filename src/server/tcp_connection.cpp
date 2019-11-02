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
  std::cout << "New client connected" << std::endl;

  Write("hello");
}

void TcpConnection::Write(const std::string& message) {
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
