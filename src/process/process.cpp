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

}  // namespace process
