// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_PAXOS_LEADER_HPP_
#define INCLUDE_PAXOS_LEADER_HPP_

#include <memory>
#include <unordered_map>

#include "paxos/commander.hpp"
#include "paxos/process.hpp"
#include "paxos/scout.hpp"
#include "proto/messages.pb.h"

namespace paxos {

class Leader : public Process {
 public:
  Leader(
      common::SharedQueue<Message>& message_queue,
      common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
          dispatch_queue,
      std::string& server_name);

  void Run() override;

  void Handle(Message&& message) override;

 private:
  void HandleProposal(Proposal&& p, const std::string& from);
  void HandleAdopted(Adopted&& a, const std::string& from);
  void HandleP2B(Message&& m, const std::string& from);

  std::shared_ptr<paxos::Scout> scout_;
  common::SharedQueue<Message> scout_message_queue_;

  // Track each spawned commander, along with a SharedQueue for message passing
  // to that commander. Commanders are always spawned for a specific slot
  // number, and multiple commanders may be running at once.
  std::unordered_map<int, paxos::Commander> commanders_;
  std::unordered_map<int, std::shared_ptr<common::SharedQueue<Message>>>
      commander_message_queue_;

  BallotNumber ballot_number_;
  bool active_;
  std::unordered_map<int, Command> proposals_;
};

}  // namespace paxos

#endif  // INCLUDE_PAXOS_LEADER_HPP_
