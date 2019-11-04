// Copyright 2019 Lukas Joswiak

#include "server/tcp_server.hpp"

#include <iostream>

const std::array<uint16_t, 2> kServerPorts = {1111, 1112};

TcpServer::TcpServer(boost::asio::io_context& io_context, uint16_t port)
    : io_context_(io_context),
      acceptor_(io_context, boost::asio::ip::tcp::endpoint(
          boost::asio::ip::tcp::v4(), port)),
      resolver_(io_context),
      connection_manager_(),
      handler_(connection_manager_) {
  for (auto server_port : kServerPorts) {
    if (server_port != port) {
      auto endpoints = resolver_.resolve(
          "localhost",
          std::to_string(server_port));
      StartConnect(endpoints, endpoints.begin());
    }
  }

  StartAccept();
}

void TcpServer::StartConnect(
    boost::asio::ip::tcp::resolver::results_type endpoints,
    boost::asio::ip::tcp::resolver::results_type::iterator endpoint_iter) {
  if (endpoint_iter != endpoints.end()) {
    std::cout << "Connecting to " << endpoint_iter->endpoint() << std::endl;

    std::shared_ptr<TcpConnection> connection = TcpConnection::Create(
        io_context_, handler_);
    connection->socket().async_connect(endpoint_iter->endpoint(),
                                       std::bind(&TcpServer::HandleConnect,
                                                 this, std::placeholders::_1,
                                                 connection, endpoints,
                                                 endpoint_iter));
  }
}

void TcpServer::HandleConnect(
    const boost::system::error_code& error,
    std::shared_ptr<TcpConnection> connection,
    boost::asio::ip::tcp::resolver::results_type endpoints,
    boost::asio::ip::tcp::resolver::results_type::iterator endpoint_iter) {
  if (!connection->socket().is_open()) {
    std::cerr << "Connect timed out" << std::endl;
    StartConnect(endpoints, ++endpoint_iter);
  } else if (error) {
    std::cerr << "Connect error: " << error.message() << std::endl;
    connection->socket().close();
    StartConnect(endpoints, ++endpoint_iter);
  } else {
    std::cout << "Connected to " << endpoint_iter->endpoint()  << std::endl;
    auto re = connection->socket().remote_endpoint().port();
    std::cout << "remote addr " << re << std::endl;
    connection_manager_.Add(connection);
  }
}

void TcpServer::StartAccept() {
  std::shared_ptr<TcpConnection> new_connection = TcpConnection::Create(
      io_context_, handler_);

  acceptor_.async_accept(new_connection->socket(),
                         std::bind(&TcpServer::HandleAccept, this,
                                   new_connection, std::placeholders::_1));
}

void TcpServer::HandleAccept(std::shared_ptr<TcpConnection> new_connection,
                             const boost::system::error_code& error) {
  if (!error) {
    new_connection->Start();
  }

  StartAccept();
}
