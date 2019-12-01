// Copyright 2019 Lukas Joswiak

#include <memory>

#include "server/handler.hpp"

#include <google/protobuf/any.pb.h>

Handler::Handler(ConnectionManager& connection_manager)
    : connection_manager_(connection_manager) {}

void Handler::Handle(const std::string& message,
                     std::shared_ptr<TcpConnection> connection) {
  google::protobuf::Any any;
  any.ParseFromString(message);

  Handle(any, connection);
}

void Handler::Handle(const google::protobuf::Any& message,
                     std::shared_ptr<TcpConnection> connection) {
  Heartbeat hb;
  Request r;
  if (message.UnpackTo(&hb)) {
    HandleHeartbeat(hb, connection);
  } else if (message.UnpackTo(&r)) {
    HandleRequest(r, connection);
  }
}

void Handler::HandleHeartbeat(
    Heartbeat& hb, std::shared_ptr<TcpConnection> connection) {
  std::cout << "Received Heartbeat" << std::endl;
  connection->set_endpoint_name(hb.server_name());
  connection_manager_.Add(connection);
}

void Handler::HandleRequest(
    Request& r, std::shared_ptr<TcpConnection> connection) {
  std::cout << "Received Request from client " << connection->endpoint_name() << std::endl;
  std::cout << "  key: " << r.key() << ", value: " << r.value() << std::endl;
}

void Handler::Broadcast(const google::protobuf::Any& message) {
  // Deliver to remote replicas.
  connection_manager_.Broadcast(message);

  // Call handler for local replica.
  Handle(message, nullptr);
}

void Handler::Disconnect(std::shared_ptr<TcpConnection> connection) {
  connection_manager_.Remove(connection);
}
