// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_PAXOS_ACCEPTOR_HPP_
#define INCLUDE_PAXOS_ACCEPTOR_HPP_

#include "paxos/process.hpp"

namespace paxos {

class Acceptor : public Process {
 public:
  explicit Acceptor(common::SharedQueue<int>& message_queue,
                    common::SharedQueue<int>& dispatch_queue);

  virtual void Handle(int message);
};

}  // namespace paxos

#endif  // INCLUDE_PAXOS_ACCEPTOR_HPP_
