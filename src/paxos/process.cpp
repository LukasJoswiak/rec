// Copyright 2019 Lukas Joswiak

#include "paxos/process.hpp"

#include <iostream>
#include <thread>

namespace paxos {

Process::Process(common::SharedQueue<int>& message_queue,
                 common::SharedQueue<int>& dispatch_queue)
    : message_queue_(message_queue),
      dispatch_queue_(dispatch_queue) {}

void Process::Run() {
  while (1) {
    int front = message_queue_.front();
    message_queue_.pop();
    Handle(front);
  }
}

}  // namespace paxos
