// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_PAXOS_LEADER_HPP_
#define INCLUDE_PAXOS_LEADER_HPP_

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

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
      std::string& address);

  void Run() override;

  void Handle(Message&& message) override;

 private:
  void HandleProposal(Proposal&& p, const std::string& from);
  void HandleAdopted(Adopted&& a, const std::string& from);
  void HandlePreempted(Preempted&& p, const std::string& from);

  // Handlers used to pass messages on to scout or commander.
  void HandleP1B(Message&& m, const std::string& from);
  void HandleP2B(Message&& m, const std::string& from);

  // Spawns and runs a commander on a thread for the given slot number and
  // command.
  void SpawnCommander(int slot_number, Command command);

  std::shared_ptr<paxos::Scout> scout_;
  common::SharedQueue<Message> scout_message_queue_;

  // Track each spawned commander, along with a SharedQueue for message passing
  // to that commander. Commanders are always spawned for a specific slot
  // number, and multiple commanders may be running at once.
  std::unordered_map<int, paxos::Commander> commanders_;
  std::unordered_map<int, std::shared_ptr<common::SharedQueue<Message>>>
      commander_message_queue_;

  std::string address_;
  BallotNumber ballot_number_;
  bool active_;

  // Map of slot number -> Command proposed for the slot.
  std::unordered_map<int, Command> proposals_;
};

}  // namespace paxos

#endif  // INCLUDE_PAXOS_LEADER_HPP_
