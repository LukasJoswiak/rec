// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_PAXOS_REPLICA_HPP_
#define INCLUDE_PAXOS_REPLICA_HPP_

#include <string>
#include <unordered_map>
#include <queue>

#include "paxos/process.hpp"
#include "proto/messages.pb.h"

namespace paxos {

class Replica : public Process {
 public:
  Replica(common::SharedQueue<Message>& message_queue,
          common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
              dispatch_queue);

  void Handle(Message&& message) override;

 private:
  void HandleRequest(Request&& r, const std::string& from);

  void Propose();

  int slot_in_;
  int slot_out_;
  std::queue<Request> requests_;
  std::unordered_map<int, Proposal> proposals_;
  std::unordered_map<int, Decision> decisions_;
};

}  // namespace paxos

#endif  // INCLUDE_PAXOS_REPLICA_HPP_
