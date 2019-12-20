// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_PAXOS_ACCEPTOR_HPP_
#define INCLUDE_PAXOS_ACCEPTOR_HPP_

#include <set>

#include "paxos/process.hpp"
#include "proto/messages.pb.h"

namespace paxos {

class Acceptor : public Process {
 public:
  Acceptor(
      common::SharedQueue<Message>& message_queue,
      common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
          dispatch_queue);

  virtual void Handle(Message&& message);

 private:
  void HandleP1A(P1A&& p, const std::string& from);
  void HandleP2A(P2A&& p, const std::string& from);

  BallotNumber ballot_number_;
  std::set<PValue> accepted_;
};

}  // namespace paxos

#endif  // INCLUDE_PAXOS_ACCEPTOR_HPP_
