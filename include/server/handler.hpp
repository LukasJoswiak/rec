// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_SERVER_HANDLER_HPP_
#define INCLUDE_SERVER_HANDLER_HPP_

#include <string>

#include <boost/asio.hpp>

#include "server/connection_manager.hpp"
// class ConnectionManager;

// Message handler for messages received on the replica.
class Handler {
 public:
  Handler(ConnectionManager& connection_manager);
  Handler(const Handler& other) = delete;
  Handler& operator=(const Handler& other) = delete;

  // Handler for incoming messages on a socket. Dispatches message to
  // appropriate handler.
  void Handle(const std::string& message, boost::asio::ip::tcp::endpoint from);

 private:
  // Reference to the connection manager, used to queue messages for delivery.
  ConnectionManager& connection_manager_;
};

#endif  // INCLUDE_SERVER_HANDLER_HPP_
