// Copyright 2019 Lukas Joswiak

#include "server/tcp_server.hpp"

#include <iostream>
#include <thread>

#include "proto/heartbeat.pb.h"

const std::array<uint16_t, 3> kServerPorts = {1111, 1112, 1113};
const std::array<std::string, 3> kServerNames =
    {"server1", "server2", "server3"};

TcpServer::TcpServer(
    boost::asio::io_context& io_context, std::string&& name, uint16_t port)
    : io_context_(io_context),
      acceptor_(io_context, boost::asio::ip::tcp::endpoint(
          boost::asio::ip::tcp::v4(), port)),
      resolver_(io_context),
      name_(name),
      connection_manager_(name) {
  for (int i = 0; i < kServerPorts.size(); ++i) {
    auto server_port = kServerPorts.at(i);
    auto server_name = kServerNames.at(i);
    if (server_port != port) {
      auto endpoints = resolver_.resolve(
          "localhost",
          std::to_string(server_port));
      StartConnect(endpoints, endpoints.begin(), server_name);
    }
  }

  StartAccept();
}

void TcpServer::StartConnect(
    boost::asio::ip::tcp::resolver::results_type endpoints,
    boost::asio::ip::tcp::resolver::results_type::iterator endpoint_iter,
    std::string& endpoint_name) {
  if (endpoint_iter != endpoints.end()) {
    std::cout << "Connecting to " << endpoint_iter->endpoint() << std::endl;

    std::shared_ptr<TcpConnection> connection = TcpConnection::Create(
        io_context_, connection_manager_, endpoint_name);
    connection->socket().async_connect(endpoint_iter->endpoint(),
                                       std::bind(&TcpServer::HandleConnect,
                                                 this, std::placeholders::_1,
                                                 connection, endpoints,
                                                 endpoint_iter, endpoint_name));
  }
}

void TcpServer::HandleConnect(
    const boost::system::error_code& error,
    std::shared_ptr<TcpConnection> connection,
    boost::asio::ip::tcp::resolver::results_type endpoints,
    boost::asio::ip::tcp::resolver::results_type::iterator endpoint_iter,
    std::string& endpoint_name) {
  if (!connection->socket().is_open()) {
    StartConnect(endpoints, ++endpoint_iter, endpoint_name);
  } else if (error) {
    connection->socket().close();
    StartConnect(endpoints, ++endpoint_iter, endpoint_name);
  } else {
    std::cout << "Connected to " << endpoint_iter->endpoint()  << std::endl;
    connection_manager_.Add(connection);

    Message message;
    message.set_type(Message_MessageType_HEARTBEAT);
    message.set_from(name_);

    connection_manager_.Deliver(message, endpoint_name);
  }
}

void TcpServer::StartAccept() {
  std::shared_ptr<TcpConnection> new_connection = TcpConnection::Create(
      io_context_, connection_manager_, "");

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
