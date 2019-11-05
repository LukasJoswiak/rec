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

void ConnectionManager::Deliver(boost::asio::ip::tcp::endpoint to,
                                const google::protobuf::Any& message) {
  for (auto connection : connections_) {
    if (connection->socket().remote_endpoint() == to) {
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
    std::cout << "  " << connection->socket().local_endpoint() << " -> "
              << connection->socket().remote_endpoint() << std::endl;
  }
}
