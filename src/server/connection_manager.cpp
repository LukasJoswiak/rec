// Copyright 2019 Lukas Joswiak

#include <google/protobuf/any.pb.h>

#include "proto/heartbeat.pb.h"
#include "server/connection_manager.hpp"

ConnectionManager::ConnectionManager(std::string& server_name)
    : handler_(*this, server_name) {}

void ConnectionManager::Add(std::shared_ptr<TcpConnection> connection) {
  if (connections_.count(connection) != 0) {
    return;
  }

  connections_.insert(connection);
  connection->Start();

  PrintManagedConnections();
}

void ConnectionManager::Remove(std::shared_ptr<TcpConnection> connection) {
  auto it = connections_.find(connection);
  if (it == connections_.end()) {
    return;
  }
  connections_.erase(it);

  PrintManagedConnections();
}

void ConnectionManager::Deliver(const google::protobuf::Any& message,
                                const std::string& endpoint) {
  std::string serialized;
  message.SerializeToString(&serialized);
  for (auto connection : connections_) {
    if (connection->endpoint_name() == endpoint) {
      connection->StartWrite(serialized);
      return;
    }
  }

  // TODO: If the TCP connection to a remote server was dropped, don't want
  // to attempt local delivery.
  // Attempt local delivery.
  handler_.Handle(message, endpoint);
}

void ConnectionManager::Broadcast(const google::protobuf::Any& message,
                                  const std::string& from) {
  std::string serialized;
  message.SerializeToString(&serialized);
  for (auto connection : connections_) {
    connection->StartWrite(serialized);
  }

  // Deliver message locally in addition to sending over the network.
  handler_.Handle(message, from);
}

void ConnectionManager::Handle(const std::string& message,
                               std::shared_ptr<TcpConnection> connection) {
  // Special case heartbeat messages to set up the connection. All other
  // messages will be routed to the message handler.
  google::protobuf::Any any;
  any.ParseFromString(message);
  Heartbeat hb;
  if (any.UnpackTo(&hb)) {
    connection->set_endpoint_name(hb.server_name());
    Add(connection);
  } else {
    handler_.Handle(message, connection->endpoint_name());
  }
}

void ConnectionManager::PrintManagedConnections() {
  std::cout << "ConnectionManager (" << connections_.size()
            << " connections)" << std::endl;
  for (auto connection : connections_) {
    std::cout << "  localhost -> " << connection->endpoint_name() << std::endl;
  }
}
