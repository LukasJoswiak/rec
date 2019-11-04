// Copyright 2019 Lukas Joswiak

#include "server/connection_manager.hpp"

ConnectionManager::ConnectionManager() { }

void ConnectionManager::Add(std::shared_ptr<TcpConnection> connection) {
  connections_.insert(connection);
  connection->Start();
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
