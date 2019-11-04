// Copyright 2019 Lukas Joswiak

#include "server/tcp_connection.hpp"

#include <iostream>

#include <google/protobuf/any.pb.h>

// Include the Handler class explicitly to allow symbol lookup to succeed.
#include "server/handler.hpp"
#include "proto/heartbeat.pb.h"

std::shared_ptr<TcpConnection> TcpConnection::Create(
    boost::asio::io_context& io_context, Handler& handler) {
  return std::shared_ptr<TcpConnection>(new TcpConnection(io_context, handler));
}

TcpConnection::TcpConnection(
    boost::asio::io_context& io_context, Handler& handler)
    : socket_(io_context),
      handler_(handler) {}

void TcpConnection::Start() {
  StartRead();

  Heartbeat hb;
  hb.set_server_name("server");
  google::protobuf::Any any;
  any.PackFrom(hb);

  std::string message;
  any.SerializeToString(&message);

  StartWrite(message);
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
    handler_.Handle(input_buffer_, socket_.remote_endpoint());

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
