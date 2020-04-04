#include "server/connection_manager.hpp"
#include "server/servers.hpp"

ConnectionManager::ConnectionManager(std::string& server_name)
    : server_name_(server_name),
      environment_(*this, server_name) {
  environment_.Start();      
}

void ConnectionManager::Shutdown() {
  for (const auto& client : clients_) {
    client->Stop();
  }

  for (const auto& server : servers_) {
    server->Stop();
  }

  clients_.clear();
  servers_.clear();
}

void ConnectionManager::AddClientConnection(
    std::shared_ptr<TcpConnection> connection) {
  if (clients_.count(connection) != 0) {
    return;
  }

  clients_.insert(connection);

  PrintManagedConnections();
}

void ConnectionManager::RemoveClientConnection(
    std::shared_ptr<TcpConnection> connection) {
  auto it = clients_.find(connection);
  if (it == clients_.end()) {
    return;
  }
  clients_.erase(it);

  PrintManagedConnections();
}

void ConnectionManager::AddServerConnection(
    std::shared_ptr<TcpConnection> connection) {
  if (servers_.count(connection) != 0) {
    return;
  }

  // Check if a connection to this server already exists.
  bool connection_exists = false;
  for (auto server : servers_) {
    if (server->endpoint() == connection->endpoint()) {
      connection_exists = true;
      break;
    }
  }

  if (!connection_exists) {
    servers_.insert(connection);
    PrintManagedConnections();
  }
}

void ConnectionManager::RemoveServerConnection(
    std::shared_ptr<TcpConnection> connection) {
  auto it = servers_.find(connection);
  if (it == servers_.end()) {
    return;
  }
  servers_.erase(it);

  PrintManagedConnections();
}

void ConnectionManager::RemoveConnection(std::shared_ptr<TcpConnection> connection) {
  RemoveClientConnection(connection);
  RemoveServerConnection(connection);
}

void ConnectionManager::Deliver(const Message& message,
                                const std::string& endpoint) {
  if (server_name_ == endpoint) {
    // Attempt local delivery.
    environment_.Handle(message);
    return;
  }

  // Attempt delivery to a server.
  for (auto connection : servers_) {
    if (connection->endpoint() == endpoint) {
      connection->Write(message);
      return;
    }
  }

  // Attempt delivery to a client.
  for (auto connection : clients_) {
    if (connection->endpoint() == endpoint ||
        (kClosedLoopTest && connection->endpoint() == "localhost" &&
        endpoint.substr(0, 6) == "client")) {
      connection->Write(message);
      return;
    }
  }
}

void ConnectionManager::Broadcast(const Message& message) {
  for (auto connection : servers_) {
    connection->Write(message);
  }

  // Deliver message locally in addition to sending over the network.
  environment_.Handle(message);
}

void ConnectionManager::Handle(const Message& message,
                               std::shared_ptr<TcpConnection> connection) {
  // Special case setup messages to set up the connection and associate TCP
  // connections with specific servers. All other messages will be routed to
  // the message handler.
  // TODO: do I still need this special setup message?
  if (message.type() == Message_MessageType_SETUP) {
    connection->set_endpoint(message.from());
    AddServerConnection(connection);
  } else if (message.type() == Message_MessageType_UNKNOWN) {
    std::cout << "Unknown message type from " << message.from()
              << ", dropping message..." << std::endl;
  } else {
    // If a request was received from a client, identify the connection as
    // a client connection and begin tracking it.
    // TODO: Only do this once.
    if (message.type() == Message_MessageType_REQUEST) {
      connection->set_endpoint(message.from());
      AddClientConnection(connection);
    }

    environment_.Handle(message);
  }
}

void ConnectionManager::PrintManagedConnections() {
  std::cout << "ConnectionManager (" << servers_.size()
            << " server connections)" << std::endl;
  for (auto connection : servers_) {
    std::cout << "  localhost -> " << connection->endpoint() << std::endl;
  }
}
