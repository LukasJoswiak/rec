// Copyright 2019 Lukas Joswiak

#include "proto/heartbeat.pb.h"
#include "server/connection_manager.hpp"

ConnectionManager::ConnectionManager(std::string& server_name)
    : environment_(*this, server_name) {}

void ConnectionManager::AddClient(std::shared_ptr<TcpConnection> connection) {
  if (clients_.count(connection) != 0) {
    return;
  }

  clients_.insert(connection);

  PrintManagedConnections();
}

void ConnectionManager::RemoveClient(
    std::shared_ptr<TcpConnection> connection) {
  auto it = clients_.find(connection);
  if (it == clients_.end()) {
    return;
  }
  clients_.erase(it);

  PrintManagedConnections();
}

void ConnectionManager::AddServer(std::shared_ptr<TcpConnection> connection) {
  if (servers_.count(connection) != 0) {
    return;
  }

  servers_.insert(connection);
  connection->Start();

  PrintManagedConnections();

  // For now, begin Paxos processes when two connections (a quorum) are active.
  // servers_ only stores remote connections, so a size of one really means two
  // active connections (counting the local server).
  if (servers_.size() >= 1) {
    environment_.Start();
  }
}

void ConnectionManager::RemoveServer(
    std::shared_ptr<TcpConnection> connection) {
  auto it = servers_.find(connection);
  if (it == servers_.end()) {
    return;
  }
  servers_.erase(it);

  PrintManagedConnections();
}

void ConnectionManager::Remove(std::shared_ptr<TcpConnection> connection) {
  RemoveClient(connection);
  RemoveServer(connection);
}

void ConnectionManager::Deliver(const Message& message,
                                const std::string& endpoint) {
  std::string serialized;
  message.SerializeToString(&serialized);

  // Attempt delivery to a server.
  for (auto connection : servers_) {
    if (connection->endpoint_name() == endpoint) {
      connection->StartWrite(serialized);
      return;
    }
  }

  // Attempt delivery to a client.
  for (auto connection : clients_) {
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
  for (auto connection : servers_) {
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
    AddServer(connection);
  } else if (message.type() == Message_MessageType_UNKNOWN) {
    std::cout << "Unknown message type from " << message.from()
              << ", dropping message..." << std::endl;
  } else {
    // If a request was received from a client, identify the connection as
    // a client connection and begin tracking it.
    if (message.type() == Message_MessageType_REQUEST) {
      connection->set_endpoint_name(message.from());
      AddClient(connection);
    }

    environment_.Handle(message);
  }
}

void ConnectionManager::PrintManagedConnections() {
  std::cout << "ConnectionManager (" << servers_.size()
            << " server connections)" << std::endl;
  for (auto connection : servers_) {
    std::cout << "  localhost -> " << connection->endpoint_name() << std::endl;
  }
}
