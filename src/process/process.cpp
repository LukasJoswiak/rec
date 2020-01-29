// Copyright 2019 Lukas Joswiak

#include "process/process.hpp"

#include <thread>

namespace process {

Process::Process(
    common::SharedQueue<Message>& message_queue,
    common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
        dispatch_queue)
    : message_queue_(message_queue),
      dispatch_queue_(dispatch_queue),
      exit_(false) {}

Process::~Process() {}

void Process::Run() {
  while (1) {
    auto front = message_queue_.front();
    message_queue_.pop();

    // Call handler in derived class.
    Handle(std::move(front));

    if (exit_) {
      return;
    }
  }
}

int Process::CompareBallotNumbers(const BallotNumber& b1,
                                  const BallotNumber& b2) {
  if (b1.number() != b2.number()) {
    return b2.number() - b1.number();
  } else {
    if (b1.address().size() == 0 && b2.address().size() > 0) {
      return 1;
    }
    return b2.address().compare(b1.address());
  }
}

}  // namespace process
