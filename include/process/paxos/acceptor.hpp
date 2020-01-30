// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_PAXOS_ACCEPTOR_HPP_
#define INCLUDE_PAXOS_ACCEPTOR_HPP_

#include <string>
#include <unordered_set>
#include <utility>

#include "process/paxos/compare.hpp"
#include "process/paxos/paxos_process.hpp"
#include "proto/messages.pb.h"

namespace process {
namespace paxos {

class Acceptor : public PaxosProcess {
 public:
  Acceptor(
      common::SharedQueue<Message>& message_queue,
      common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
          dispatch_queue,
      std::string& address);

  void Handle(Message&& message) override;

 private:
  void HandleP1A(P1A&& p, const std::string& from);
  void HandleP2A(P2A&& p, const std::string& from);

  std::string address_;

  BallotNumber ballot_number_;
  std::unordered_set<PValue, PValueHash, PValueEqualTo> accepted_;
};

}  // namespace paxos
}  // namespace process

#endif  // INCLUDE_PAXOS_ACCEPTOR_HPP_
