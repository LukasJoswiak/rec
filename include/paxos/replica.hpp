// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_PAXOS_REPLICA_HPP_
#define INCLUDE_PAXOS_REPLICA_HPP_

#include <set>
#include <vector>

#include "proto/messages.pb.h"

namespace paxos {

class Replica {
 public:
  Replica();

  void Run();

 private:
  int slot_in_;
  int slot_out_;
  std::vector<Request> requests_;
  std::set<Request> proposals_;
  std::set<Decision> decisions_;
};

}  // namespace paxos

#endif  // INCLUDE_PAXOS_REPLICA_HPP_
