#ifndef INCLUDE_SERVER_CONNECTION_MANAGER_HPP_
#define INCLUDE_SERVER_CONNECTION_MANAGER_HPP_

#include <memory>
#include <set>
#include <string>

#include "proto/messages.pb.h"
#include "spdlog/spdlog.h"
#include "server/environment.hpp"
#include "server/tcp_connection.hpp"

// Tracks open connections and provides message handling and delivery
// functionality.
class ConnectionManager {
 public:
  explicit ConnectionManager(std::string& server_name);

  // Shuts down all tracked TCP connections.
  void Shutdown();

  // Starts tracking a client connection.
  void AddClientConnection(std::shared_ptr<TcpConnection> connection);

  // Stops tracking a client connection.
  void RemoveClientConnection(std::shared_ptr<TcpConnection> connection);

  // Starts tracking a server connection.
  void AddServerConnection(std::shared_ptr<TcpConnection> connection);

  // Stops tracking a server connection.
  void RemoveServerConnection(std::shared_ptr<TcpConnection> connection);

  // Stops tracking a connection. Attempts to remove from both client and server
  // connection pools.
  void RemoveConnection(std::shared_ptr<TcpConnection> connection);

  // Attempts delivery of the message to the given endpoint. Endpoint must be an
  // active connection tracked by the connection manager or the name of this
  // server so message can be handled locally.
  void Deliver(const Message& message, const std::string& endpoint);

  // Attempts delivery of the message to all known server endpoints.
  void Broadcast(const Message& message);

  // Passes message to correct handler.
  void Handle(const Message& message,
              std::shared_ptr<TcpConnection> connection);

  // Debug function used to print the connections being managed.
  void PrintManagedConnections();

 private:
  std::set<std::shared_ptr<TcpConnection>> clients_;
  std::set<std::shared_ptr<TcpConnection>> servers_;

  std::string server_name_;

  // Message handler used to pass messages to appropriate location.
  Environment environment_;

  std::shared_ptr<spdlog::logger> logger_;
};

#endif  // INCLUDE_SERVER_CONNECTION_MANAGER_HPP_
