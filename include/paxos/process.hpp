// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_PAXOS_PROCESS_HPP_
#define INCLUDE_PAXOS_PROCESS_HPP_

#include "paxos/shared_queue.hpp"

#include "proto/messages.pb.h"

namespace paxos {

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
  // Returns a negative number if b1 > b2, a positive number if b1 < b2, or
  // zero if b1 equals b2. Ballot numbers are totally ordered and are compared
  // first using their numbers, then using the server address for tie-breakers.
  int CompareBallotNumbers(const BallotNumber& b1, const BallotNumber& b2);

  // As messages for the process are received, they are added to this queue.
  common::SharedQueue<Message>& message_queue_;

  // Messages added to this queue will be delivered to the appropriate server.
  common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
      dispatch_queue_;
};

}  // namespace paxos

#endif  // INCLUDE_PAXOS_PROCESS_HPP_
