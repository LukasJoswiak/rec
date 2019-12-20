// Copyright 2019 Lukas Joswiak

#include "paxos/acceptor.hpp"

#include <iostream>

namespace paxos {

Acceptor::Acceptor(
    common::SharedQueue<Message>& message_queue,
    common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
        dispatch_queue)
    : Process(message_queue, dispatch_queue) {}

void Acceptor::Handle(Message&& message) {
  if (message.type() == Message_MessageType_P1A) {
    P1A p;
    message.message().UnpackTo(&p);
    HandleP1A(std::move(p), message.from());
  } else if (message.type() == Message_MessageType_P2A) {
    P2A p;
    message.message().UnpackTo(&p);
    HandleP2A(std::move(p), message.from());
  }
}

void Acceptor::HandleP1A(P1A&& p, const std::string& from) {
  std::cout << "Received P1A from " << from << std::endl;
}

void Acceptor::HandleP2A(P2A&& p, const std::string& from) {
  std::cout << "Received P2A from " << from << std::endl;
}

}  // namespace paxos
