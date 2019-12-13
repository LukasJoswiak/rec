// Copyright 2019 Lukas Joswiak

#include "paxos/acceptor.hpp"

#include <thread>

namespace paxos {

Acceptor::Acceptor(common::SharedQueue<int>& queue) : queue_(queue) {
  std::cout << "Acceptor created" << std::endl;
}

void Acceptor::Run() {
  std::cout << "Acceptor running on thread " << std::this_thread::get_id()
            << std::endl;

  queue_.push(105);
}

}  // namespace paxos
