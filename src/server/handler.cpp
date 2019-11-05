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
  std::cout << "Heartbeat server name: " << hb.server_name() << std::endl;

  /*
  // Example of how to reply.
  Heartbeat hb2;
  hb2.set_server_name("hello");
  google::protobuf::Any any2;
  any2.PackFrom(hb2);
  connection_manager_.Deliver(from, any2);
  */
  connection_manager_.Add(connection);
}
