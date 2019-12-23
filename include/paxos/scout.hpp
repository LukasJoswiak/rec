// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_PAXOS_SCOUT_HPP_
#define INCLUDE_PAXOS_SCOUT_HPP_

#include <string>
#include <unordered_set>
#include <utility>

#include "paxos/compare.hpp"
#include "paxos/process.hpp"
#include "proto/messages.pb.h"

namespace paxos {

class Scout : public Process {
 public:
  Scout(
      common::SharedQueue<Message>& message_queue,
      common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
          dispatch_queue,
      std::string& leader, BallotNumber& ballot_number);
  ~Scout() override;

  void Run() override;

  void Handle(Message&& message) override;

  BallotNumber& ballot_number() {
    return ballot_number_;
  }

 private:
  void HandleP1B(P1B&& p, const std::string& from);

  std::string leader_;
  BallotNumber ballot_number_;
  std::unordered_set<PValue, PValueHash, PValueEqualTo> pvalues_;
  std::unordered_set<std::string> received_from_;
};

}  // namespace paxos

#endif  // INCLUDE_PAXOS_SCOUT_HPP_
