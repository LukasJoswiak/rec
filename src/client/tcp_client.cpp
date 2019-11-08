// Copyright 2019 Lukas Joswiak

#include "client/tcp_client.hpp"

#include <iostream>

TcpClient::TcpClient(boost::asio::io_context& io_context)
    : socket_(io_context) {}

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
  }
}

