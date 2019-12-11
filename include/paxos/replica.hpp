// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_PAXOS_REPLICA_HPP_
#define INCLUDE_PAXOS_REPLICA_HPP_

#include <set>
#include <vector>

#include "proto/messages.pb.h"
#include "paxos/shared_queue.hpp"

namespace paxos {

class Replica {
 public:
  explicit Replica(common::SharedQueue<int>& queue);

  // Begin handling messages.
  void Run();

  void Test() {
    queue_.push(100);
  }

 private:
  int slot_in_;
  int slot_out_;
  std::vector<Request> requests_;
  std::set<Request> proposals_;
  std::set<Decision> decisions_;

  common::SharedQueue<int>& queue_;
};

}  // namespace paxos

#endif  // INCLUDE_PAXOS_REPLICA_HPP_
