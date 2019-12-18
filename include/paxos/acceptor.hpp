// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_PAXOS_ACCEPTOR_HPP_
#define INCLUDE_PAXOS_ACCEPTOR_HPP_

#include "paxos/process.hpp"
#include "proto/messages.pb.h"

namespace paxos {

class Acceptor : public Process {
 public:
  Acceptor(
      common::SharedQueue<Message>& message_queue,
      common::SharedQueue<std::pair<std::string, Message>>& dispatch_queue);

  virtual void Handle(Message&& message);
};

}  // namespace paxos

#endif  // INCLUDE_PAXOS_ACCEPTOR_HPP_
