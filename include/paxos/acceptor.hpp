// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_PAXOS_ACCEPTOR_HPP_
#define INCLUDE_PAXOS_ACCEPTOR_HPP_

#include "paxos/shared_queue.hpp"

namespace paxos {

class Acceptor {
 public:
  explicit Acceptor(common::SharedQueue<int>& queue);

  // Begin handling messages.
  void Run();

 private:
  common::SharedQueue<int>& queue_;
};

}  // namespace paxos

#endif  // INCLUDE_PAXOS_ACCEPTOR_HPP_
