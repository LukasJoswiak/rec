// Copyright 2019 Lukas Joswiak

#include "paxos/replica.hpp"

#include <thread>

namespace paxos {

Replica::Replica() : slot_in_(1), slot_out_(1) {
  std::cout << "Replica created" << std::endl;
}

void Replica::Run() {
  std::cout << "Replica running on thread " << std::this_thread::get_id()
            << std::endl;
}

}  // namespace paxos
