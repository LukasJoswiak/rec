// Copyright 2019 Lukas Joswiak

#include "server/connection_manager.hpp"

ConnectionManager::ConnectionManager() { }

void ConnectionManager::Add(std::shared_ptr<TcpConnection> connection) {
  connections_.insert(connection);
  connection->Start();
}
