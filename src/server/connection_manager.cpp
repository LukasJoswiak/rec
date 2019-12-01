// Copyright 2019 Lukas Joswiak

#include "server/connection_manager.hpp"

ConnectionManager::ConnectionManager() { }

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
}

void ConnectionManager::PrintManagedConnections() {
  std::cout << "ConnectionManager (" << connections_.size()
            << " connections)" << std::endl;
  for (auto connection : connections_) {
    std::cout << "  localhost -> " << connection->endpoint_name() << std::endl;
  }
}
