// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_SERVER_HANDLER_HPP_
#define INCLUDE_SERVER_HANDLER_HPP_

#include <memory>
#include <string>

#include <boost/asio.hpp>

#include "proto/heartbeat.pb.h"
#include "proto/messages.pb.h"
#include "server/connection_manager.hpp"

// Message handler for messages received on the replica.
class Handler {
 public:
  explicit Handler(ConnectionManager& connection_manager);
  Handler(const Handler& other) = delete;
  Handler& operator=(const Handler& other) = delete;

  // Parses message into appropriate message type and calls the correct handler.
  void Handle(const std::string& message,
              std::shared_ptr<TcpConnection> connection);

  // Parses message and calls correct handler.
  void Handle(const google::protobuf::Any& message,
              std::shared_ptr<TcpConnection> connection);

  // Signals a connection has been closed and should stop being tracked.
  void Disconnect(std::shared_ptr<TcpConnection> connection);

 private:
  // Broadcasts the message locally and to all remote connections.
  void Broadcast(const google::protobuf::Any& message);

  void HandleHeartbeat(Heartbeat& hb,
                       std::shared_ptr<TcpConnection> connection);

  void HandleRequest(Request& r,
                       std::shared_ptr<TcpConnection> connection);

  // Reference to the connection manager, used to queue messages for delivery.
  ConnectionManager& connection_manager_;
};

#endif  // INCLUDE_SERVER_HANDLER_HPP_
