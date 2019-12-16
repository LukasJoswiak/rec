// Copyright 2019 Lukas Joswiak

#include "paxos/acceptor.hpp"

#include <iostream>

namespace paxos {

Acceptor::Acceptor(common::SharedQueue<int>& message_queue,
                   common::SharedQueue<int>& dispatch_queue)
    : Process(message_queue, dispatch_queue) {}

void Acceptor::Handle(int message) {
  std::cout << "Acceptor received message: " << message << std::endl;
}

}  // namespace paxos
