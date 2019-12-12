// Copyright 2019 Lukas Joswiak

#include "server/handler.hpp"

#include <memory>

#include "paxos/replica.hpp"

Handler::Handler(ConnectionManager& manager, std::string& server_name)
    : environment_(manager, server_name) {}

void Handler::Handle(const std::string& message, const std::string& from) {
  google::protobuf::Any any;
  any.ParseFromString(message);

  Handle(any, from);
}

void Handler::Handle(const google::protobuf::Any& message,
                     const std::string& from) {
  Request r;
  if (message.UnpackTo(&r)) {
    environment_.HandleRequest(r, from);
  }
}
