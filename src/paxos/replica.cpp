// Copyright 2019 Lukas Joswiak

#include "paxos/replica.hpp"

namespace paxos {

Replica::Replica(common::SharedQueue<int>& message_queue,
                 common::SharedQueue<int>& dispatch_queue)
    : Process(message_queue, dispatch_queue),
      slot_in_(1),
      slot_out_(1) {}

void Replica::Handle(int message) {
  std::cout << "Replica received message: " << message << std::endl;
}

}  // namespace paxos
