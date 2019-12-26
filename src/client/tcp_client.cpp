// Copyright 2019 Lukas Joswiak

#include "client/tcp_client.hpp"

#include <iostream>

#include "proto/messages.pb.h"

TcpClient::TcpClient(boost::asio::io_context& io_context, std::string&& name)
    : socket_(io_context),
      name_(name) {}

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
    std::cerr << "Connect timed out" << std::endl;
    StartConnect(++endpoint_iter);
  } else if (error) {
    std::cerr << "Connect error: " << error.message() << std::endl;
    socket_.close();
    StartConnect(++endpoint_iter);
  } else {
    std::cout << "Connected to " << endpoint_iter->endpoint()  << std::endl;

    auto c = new Command();
    c->set_client(name_);
    c->set_sequence_number(1);
    c->set_operation(0);

    Request r;
    r.set_allocated_command(c);
    r.set_source(name_);

    Message m;
    m.set_type(Message_MessageType_REQUEST);
    m.mutable_message()->PackFrom(r);
    m.set_from(name_);

    std::string serialized;
    m.SerializeToString(&serialized);

    StartRead();
    StartWrite(serialized);
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
    std::string message(
        boost::asio::buffers_begin(input_buffer_.data()),
        boost::asio::buffers_begin(input_buffer_.data()) + bytes_transferred);
    input_buffer_.consume(bytes_transferred);
    std::cout << "Received message: " << message << std::endl;

    StartRead();
  } else {
    std::cerr << "Receive error: " << error.message() << std::endl;
    Stop();
  }
}

void TcpClient::StartWrite(const std::string& message) {
  boost::asio::async_write(
      socket_, boost::asio::buffer(message, message.size()),
      std::bind(&TcpClient::HandleWrite, this, std::placeholders::_1));
}

void TcpClient::HandleWrite(const boost::system::error_code& error) {
  if (error) {
    Stop();
  }
}
