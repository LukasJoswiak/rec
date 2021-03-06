#ifndef INCLUDE_PROCESS_PROCESS_HPP_
#define INCLUDE_PROCESS_PROCESS_HPP_

#include <string>
#include <utility>

#include "proto/messages.pb.h"
#include "spdlog/spdlog.h"
#include "process/shared_queue.hpp"

namespace process {

// Base class for behavior including receiving and sending messages. This class
// cannot be instantiated, and all derived classes must implement the Handle
// function.
class Process {
 public:
  Process(common::SharedQueue<Message>& message_queue,
          common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
              dispatch_queue);
  virtual ~Process();

  // Begin handling messages.
  virtual void Run();

  // Handle a single message received on the message queue. This function should
  // be overridden by the derived class to implement message specific
  // functionality.
  virtual void Handle(Message&& message) = 0;

 protected:
  // As messages for the process are received, they are added to this queue.
  common::SharedQueue<Message>& message_queue_;

  // Messages added to this queue will be delivered to the appropriate server.
  common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
      dispatch_queue_;

  // Set to true to tell the process to exit.
  bool exit_;

  std::shared_ptr<spdlog::logger> logger_;
};

}  // namespace process

#endif  // INCLUDE_PROCESS_PROCESS_HPP_
