// Copyright 2019 Lukas Joswiak

#include "paxos/replica.hpp"

#include <thread>

namespace paxos {

Replica::Replica(common::SharedQueue<int>& queue)
    : slot_in_(1),
      slot_out_(1),
      queue_(queue) {
  std::cout << "Replica created" << std::endl;
}

void Replica::Run() {
  std::cout << "Replica running on thread " << std::this_thread::get_id()
            << std::endl;

  queue_.push(104);
}

}  // namespace paxos
