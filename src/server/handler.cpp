// Copyright 2019 Lukas Joswiak

#include "server/handler.hpp"

#include <memory>

#include "paxos/replica.hpp"

// TODO: May be able to get rid of the Handler class

Handler::Handler(ConnectionManager& manager, std::string& server_name)
    : environment_(manager, server_name) {}

void Handler::Handle(const std::string& raw_message) {
  Message message;
  message.ParseFromString(raw_message);
  Handle(message);
}

void Handler::Handle(const Message& message) {
  auto type = message.type();
  switch (message.type()) {
    case Message_MessageType_REQUEST:
    case Message_MessageType_PROPOSAL:
      environment_.HandleReplicaMessage(message);
      break;
    case Message_MessageType_P1A:
    case Message_MessageType_P2A:
      environment_.HandleAcceptorMessage(message);
      break;
    default:
      break;
  }
}
