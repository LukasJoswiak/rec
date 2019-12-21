// Copyright 2019 Lukas Joswiak

#include "proto/heartbeat.pb.h"
#include "server/connection_manager.hpp"

ConnectionManager::ConnectionManager(std::string& server_name)
    : environment_(*this, server_name) {}

void ConnectionManager::Add(std::shared_ptr<TcpConnection> connection) {
  if (connections_.count(connection) != 0) {
    return;
  }

  connections_.insert(connection);
  connection->Start();

  PrintManagedConnections();

  // For now, begin Paxos processes when two connections (a quorum) are active.
  // connections_ only stores remote connections, so a size of one really means
  // two active connections (counting the local server).
  if (connections_.size() >= 1) {
    environment_.Start();
  }
}

void ConnectionManager::Remove(std::shared_ptr<TcpConnection> connection) {
  auto it = connections_.find(connection);
  if (it == connections_.end()) {
    return;
  }
  connections_.erase(it);

  PrintManagedConnections();
}

void ConnectionManager::Deliver(const Message& message,
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
  environment_.Handle(message);
}

void ConnectionManager::Broadcast(const Message& message) {
  std::string serialized;
  message.SerializeToString(&serialized);
  for (auto connection : connections_) {
    connection->StartWrite(serialized);
  }

  // Deliver message locally in addition to sending over the network.
  environment_.Handle(message);
}

void ConnectionManager::Handle(const std::string& raw_message,
                               std::shared_ptr<TcpConnection> connection) {
  // Special case heartbeat messages to set up the connection. All other
  // messages will be routed to the message handler.
  Message message;
  message.ParseFromString(raw_message);
  if (message.type() == Message_MessageType_HEARTBEAT) {
    connection->set_endpoint_name(message.from());
    Add(connection);
  } else if (message.type() == Message_MessageType_UNKNOWN) {
    std::cout << "Unknown message type from " << message.from()
              << ", dropping message..." << std::endl;
  } else {
    environment_.Handle(message);
  }
}

void ConnectionManager::PrintManagedConnections() {
  std::cout << "ConnectionManager (" << connections_.size()
            << " connections)" << std::endl;
  for (auto connection : connections_) {
    std::cout << "  localhost -> " << connection->endpoint_name() << std::endl;
  }
}
