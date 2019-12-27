// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_PAXOS_REPLICA_HPP_
#define INCLUDE_PAXOS_REPLICA_HPP_

#include <queue>
#include <string>
#include <unordered_map>
#include <utility>

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
  void HandleDecision(Decision&& d, const std::string& from);

  void Propose();

  // Executes the given command at slot number slot_out_, or performs a no-op
  // if the command has already been executed.
  void Execute(const Command& command);

  // Key-value store for the application.
  std::unordered_map<std::string, std::string> store_;

  int slot_in_;
  int slot_out_;
  std::queue<Command> requests_;
  std::unordered_map<int, Command> proposals_;
  std::unordered_map<int, Command> decisions_;
};

}  // namespace paxos

#endif  // INCLUDE_PAXOS_REPLICA_HPP_
