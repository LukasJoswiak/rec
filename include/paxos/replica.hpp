// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_PAXOS_REPLICA_HPP_
#define INCLUDE_PAXOS_REPLICA_HPP_

#include <set>
#include <vector>

#include "paxos/process.hpp"
#include "proto/messages.pb.h"

namespace paxos {

class Replica : public Process {
 public:
  explicit Replica(common::SharedQueue<int>& message_queue,
                   common::SharedQueue<int>& dispatch_queue);

  virtual void Handle(int message);

 private:
  int slot_in_;
  int slot_out_;
  std::vector<Request> requests_;
  std::set<Request> proposals_;
  std::set<Decision> decisions_;
};

}  // namespace paxos

#endif  // INCLUDE_PAXOS_REPLICA_HPP_
