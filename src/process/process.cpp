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
    Message message;
    if (!message_queue_.try_pop(&message)) {
      // Queue has been shutdown.
      return;
    }

    // Call handler in derived class.
    Handle(std::move(message));

    if (exit_) {
      return;
    }
  }
}

}  // namespace process
