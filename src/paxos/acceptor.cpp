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
  std::cout << "Acceptor received message" << std::endl;
}

}  // namespace paxos
