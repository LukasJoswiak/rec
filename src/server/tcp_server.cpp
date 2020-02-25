#include "server/tcp_server.hpp"

#include <iostream>
#include <thread>

#include "server/servers.hpp"

TcpServer::TcpServer(std::string&& name, uint16_t port)
    : io_context_(1),
      signals_(io_context_),
      acceptor_(io_context_, boost::asio::ip::tcp::endpoint(
          boost::asio::ip::tcp::v4(), port)),
      resolver_(io_context_),
      name_(name),
      connection_manager_(name) {
  // Set shutdown conditions.
  signals_.add(SIGINT);
  signals_.add(SIGTERM);
  signals_.async_wait(std::bind(&TcpServer::HandleStop, this,
                      std::placeholders::_1, std::placeholders::_2));

  // Attempt to open connections to other servers.
  for (int i = 0; i < kServers.size(); ++i) {
    auto server_name = std::get<0>(kServers.at(i));
    auto server_port = std::get<1>(kServers.at(i));
    if (server_name != name_) {
      auto endpoints = resolver_.resolve(
          "localhost",
          std::to_string(server_port));
      StartConnect(endpoints, endpoints.begin(), server_name);
    }
  }

  // Handle incoming connections.
  StartAccept();
}

void TcpServer::Run() {
  io_context_.run();
}

void TcpServer::StartConnect(
    boost::asio::ip::tcp::resolver::results_type endpoints,
    boost::asio::ip::tcp::resolver::results_type::iterator endpoint_iter,
    std::string& endpoint_name) {
  if (endpoint_iter != endpoints.end()) {
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
    std::cout << "Connected to " << endpoint_name << std::endl;
    connection->socket().set_option(boost::asio::ip::tcp::no_delay(true));
    connection_manager_.AddServerConnection(connection);

    Message message;
    message.set_type(Message_MessageType_SETUP);
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
  if (!acceptor_.is_open()) {
    return;
  }

  if (!error) {
    new_connection->socket().set_option(boost::asio::ip::tcp::no_delay(true));
    new_connection->Start();
  }

  StartAccept();
}

void TcpServer::HandleStop(boost::system::error_code error, int signal_number) {
  if (error) {
    std::cerr << "Error shutting down with signal " << signal_number << ": "
              << error.message() << std::endl;
  }
  acceptor_.close();
  connection_manager_.Shutdown();
  std::cout << "Server stopped" << std::endl;
}
