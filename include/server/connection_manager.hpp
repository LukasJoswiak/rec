// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_SERVER_CONNECTION_MANAGER_HPP_
#define INCLUDE_SERVER_CONNECTION_MANAGER_HPP_

#include <memory>
#include <set>

#include <google/protobuf/any.pb.h>

#include "server/tcp_connection.hpp"

// Keeps track of open connections.
class ConnectionManager {
 public:
  ConnectionManager();

  // Adds connection to managed connections; starts connection.
  void Add(std::shared_ptr<TcpConnection> connection);

  // Attempts delivery of the given message to the given endpoint. Endpoint must
  // be an active connection being tracked by the connection manager.
  void Deliver(boost::asio::ip::tcp::endpoint to,
               const google::protobuf::Any& message);

 private:
  std::set<std::shared_ptr<TcpConnection>> connections_;
};

#endif  // INCLUDE_SERVER_CONNECTION_MANAGER_HPP_
