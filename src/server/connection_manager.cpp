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

void ConnectionManager::Deliver(const std::string& endpoint,
                                const google::protobuf::Any& message) const {
  for (auto connection : connections_) {
    if (connection->endpoint_name() == endpoint) {
      std::string serialized;
      message.SerializeToString(&serialized);
      connection->StartWrite(serialized);
    }
  }
}

void ConnectionManager::PrintManagedConnections() {
  std::cout << "ConnectionManager (" << connections_.size()
            << " connections)" << std::endl;
  for (auto connection : connections_) {
    std::cout << "  localhost -> " << connection->endpoint_name() << std::endl;
  }
}
