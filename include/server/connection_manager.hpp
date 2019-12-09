// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_SERVER_CONNECTION_MANAGER_HPP_
#define INCLUDE_SERVER_CONNECTION_MANAGER_HPP_

#include <google/protobuf/any.pb.h>

#include <memory>
#include <set>
#include <string>
#include "server/tcp_connection.hpp"

// Keeps track of open connections.
class ConnectionManager {
 public:
  ConnectionManager();

  // Adds connection to managed connections; starts connection.
  void Add(std::shared_ptr<TcpConnection> connection);

  // Removes connection from managed connections.
  void Remove(std::shared_ptr<TcpConnection> connection);

  // Attempts delivery of the message to the given endpoint. Endpoint must be an
  // active connection tracked by the connection manager.
  void Deliver(const std::string& endpoint,
               const google::protobuf::Any& message) const;

  // Attempts delivery of the message to all known endpoints.
  void Broadcast(const google::protobuf::Any& message) const;

  // Debug function used to print the connections being managed.
  void PrintManagedConnections();

 private:
  std::set<std::shared_ptr<TcpConnection>> connections_;
};

#endif  // INCLUDE_SERVER_CONNECTION_MANAGER_HPP_
