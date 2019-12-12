// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_SERVER_CONNECTION_MANAGER_HPP_
#define INCLUDE_SERVER_CONNECTION_MANAGER_HPP_

#include <google/protobuf/any.pb.h>

#include <memory>
#include <set>
#include <string>

#include "server/handler.hpp"
#include "server/tcp_connection.hpp"

// Tracks open connections and provides message handling and delivery
// functionality.
class ConnectionManager {
 public:
  explicit ConnectionManager(std::string& server_name);

  // Adds connection to managed connections; starts connection.
  void Add(std::shared_ptr<TcpConnection> connection);

  // Removes connection from managed connections.
  void Remove(std::shared_ptr<TcpConnection> connection);

  // Attempts delivery of the message to the given endpoint. Endpoint must be an
  // active connection tracked by the connection manager or the name of this
  // server so message can be handled locally.
  void Deliver(const google::protobuf::Any& message,
               const std::string& endpoint);

  // Attempts delivery of the message to all known endpoints.
  void Broadcast(const google::protobuf::Any& message, const std::string& from);

  // Passes message to handler.
  void Handle(const std::string& message,
              std::shared_ptr<TcpConnection> connection);

  // Debug function used to print the connections being managed.
  void PrintManagedConnections();

 private:
  std::set<std::shared_ptr<TcpConnection>> connections_;

  // Stores a Handler instance to parse messages and pass on messages.
  Handler handler_;
};

#endif  // INCLUDE_SERVER_CONNECTION_MANAGER_HPP_
