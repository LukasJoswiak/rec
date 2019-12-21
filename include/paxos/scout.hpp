// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_PAXOS_SCOUT_HPP_
#define INCLUDE_PAXOS_SCOUT_HPP_

#include "paxos/process.hpp"
#include "proto/messages.pb.h"

namespace paxos {

class Scout : public Process {
 public:
  Scout(
      common::SharedQueue<Message>& message_queue,
      common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
          dispatch_queue,
      BallotNumber& ballot_number);

  void Handle(Message&& message) override;

 private:
  void HandleP1B(P1B&& p, const std::string& from);

  BallotNumber ballot_number_;
};

}  // namespace paxos

#endif  // INCLUDE_PAXOS_SCOUT_HPP_
