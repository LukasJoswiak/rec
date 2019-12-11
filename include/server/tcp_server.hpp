// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_SERVER_TCP_SERVER_HPP_
#define INCLUDE_SERVER_TCP_SERVER_HPP_

#include <memory>
#include <string>

#include <boost/asio.hpp>

#include "paxos/shared_queue.hpp"
#include "server/connection_manager.hpp"
#include "server/handler.hpp"
#include "server/tcp_connection.hpp"

// TCP server implementation, handling incoming and outgoing connections.
class TcpServer {
 public:
  // Initialize a new TcpServer which listens for incoming connections on the
  // given port.
  TcpServer(boost::asio::io_context& io_context, std::string&& name,
            uint16_t port);
  TcpServer(const TcpServer& other) = delete;
  TcpServer& operator=(const TcpServer& other) = delete;

 private:
  // Initiate a connection asynchronously.
  void StartConnect(
      boost::asio::ip::tcp::resolver::results_type endpoints,
      boost::asio::ip::tcp::resolver::results_type::iterator endpoint_iter,
      std::string& endpoint_name);

  // Handles the results of an asynchronous connection initiation attempt.
  void HandleConnect(
      const boost::system::error_code& error,
      std::shared_ptr<TcpConnection> connection,
      boost::asio::ip::tcp::resolver::results_type endpoints,
      boost::asio::ip::tcp::resolver::results_type::iterator endpoint_iter,
      std::string& endpoint_name);

  // Listens for new connections.
  void StartAccept();

  // Handles newly established connection and instructs server to again listen
  // for incoming connection attempts.
  void HandleAccept(std::shared_ptr<TcpConnection> new_connection,
                    const boost::system::error_code& error);

  boost::asio::io_context& io_context_;
  boost::asio::ip::tcp::acceptor acceptor_;
  boost::asio::ip::tcp::resolver resolver_;

  std::string name_;
  ConnectionManager connection_manager_;
  Handler handler_;
  common::SharedQueue<int> queue_;
};

#endif  // INCLUDE_SERVER_TCP_SERVER_HPP_
