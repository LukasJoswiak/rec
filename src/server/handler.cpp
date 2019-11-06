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

  Heartbeat hb;
  if (any.UnpackTo(&hb)) {
    HandleHeartbeat(hb, connection);
  }
}

void Handler::HandleHeartbeat(
    Heartbeat& hb, std::shared_ptr<TcpConnection> connection) {
  connection->set_endpoint_name(hb.server_name());
  connection_manager_.Add(connection);
}
