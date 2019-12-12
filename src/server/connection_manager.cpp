// Copyright 2019 Lukas Joswiak

#include <google/protobuf/any.pb.h>

#include "proto/heartbeat.pb.h"
#include "server/connection_manager.hpp"

ConnectionManager::ConnectionManager() {}

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

void ConnectionManager::Deliver(const std::string& endpoint,
                                const google::protobuf::Any& message) const {
  std::string serialized;
  message.SerializeToString(&serialized);
  for (auto connection : connections_) {
    if (connection->endpoint_name() == endpoint) {
      connection->StartWrite(serialized);
    }
  }
}

void ConnectionManager::Broadcast(const google::protobuf::Any& message) const {
  std::string serialized;
  message.SerializeToString(&serialized);
  for (auto connection : connections_) {
    connection->StartWrite(serialized);
  }

  // Use the empty string to signal message is being delivered from self.
  const std::string from = "";
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
    assert(connection->endpoint_name().size() > 0);
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
