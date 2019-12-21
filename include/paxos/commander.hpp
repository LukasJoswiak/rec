// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_PAXOS_COMMANDER_HPP_
#define INCLUDE_PAXOS_COMMANDER_HPP_

#include <unordered_set>

#include "paxos/process.hpp"
#include "proto/messages.pb.h"

namespace paxos {

class Commander : public Process {
 public:
  Commander(
      common::SharedQueue<Message>& message_queue,
      common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
          dispatch_queue,
      BallotNumber ballot_number, int slot_number, Command command);

  void Handle(Message&& message) override;

 private:
  void HandleP2B(P2B&& p, const std::string& from);

  std::unordered_set<std::string> received_from_;

  BallotNumber ballot_number_;
  int slot_number_;
  Command command_;
};

}  // namespace paxos

#endif  // INCLUDE_PAXOS_COMMANDER_HPP_