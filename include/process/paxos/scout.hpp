// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_PAXOS_SCOUT_HPP_
#define INCLUDE_PAXOS_SCOUT_HPP_

#include <string>
#include <unordered_set>
#include <utility>

#include "process/paxos/compare.hpp"
#include "process/process.hpp"
#include "proto/messages.pb.h"

namespace process {
namespace paxos {

class Scout : public Process {
 public:
  Scout(
      common::SharedQueue<Message>& message_queue,
      common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
          dispatch_queue,
      int scout_id, std::string& leader, BallotNumber& ballot_number);
  ~Scout() override;

  void Run() override;

  void Handle(Message&& message) override;

  BallotNumber& ballot_number() {
    return ballot_number_;
  }

 private:
  void HandleP1B(P1B&& p, const std::string& from);

  int scout_id_;

  std::string leader_;
  BallotNumber ballot_number_;
  std::unordered_set<PValue, PValueHash, PValueEqualTo> pvalues_;
  std::unordered_set<std::string> received_from_;
};

}  // namespace paxos
}  // namespace process

#endif  // INCLUDE_PAXOS_SCOUT_HPP_
