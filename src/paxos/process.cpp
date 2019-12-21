// Copyright 2019 Lukas Joswiak

#include "paxos/process.hpp"

#include <iostream>
#include <thread>

namespace paxos {

Process::Process(
    common::SharedQueue<Message>& message_queue,
    common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
        dispatch_queue)
    : message_queue_(message_queue),
      dispatch_queue_(dispatch_queue) {}

Process::~Process() {}

void Process::Run() {
  while (1) {
    auto front = message_queue_.front();
    message_queue_.pop();

    // Call handler in derived class.
    Handle(std::move(front));
  }
}

int Process::CompareBallotNumbers(const BallotNumber& b1,
                                  const BallotNumber& b2) {
  if (b1.number() != b2.number()) {
    return b2.number() - b1.number();
  } else {
    return b1.address().compare(b2.address());
  }
}

}  // namespace paxos
