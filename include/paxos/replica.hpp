// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_PAXOS_REPLICA_HPP_
#define INCLUDE_PAXOS_REPLICA_HPP_

#include "proto/messages.pb.h"

class Replica {
 public:
  Replica();

 private:
  int slot_in_;
  int slot_out_;
  std::vector<Request> requests_;
  std::set<Request> proposals_;
  std::set<Decision> decisions_;
};

#endif  // INCLUDE_PAXOS_REPLICA_HPP_
