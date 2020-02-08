#ifndef INCLUDE_PAXOS_REPLICA_HPP_
#define INCLUDE_PAXOS_REPLICA_HPP_

#include <optional>
#include <queue>
#include <string>
#include <unordered_map>
#include <utility>

#include "process/paxos/paxos_process.hpp"
#include "proto/internal.pb.h"
#include "proto/messages.pb.h"
#include "kvstore/application.hpp"

namespace process {
namespace paxos {

class Replica : public PaxosProcess {
 public:
  Replica(common::SharedQueue<Message>& message_queue,
          common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
              dispatch_queue,
          std::string& address);

  void Handle(Message&& message) override;

 private:
  // Internal message handlers.
  void HandleLeaderChange(LeaderChange&& l, const std::string& from);
  void HandleReconstructedProposal(ReconstructedProposal&& p,
                                   const std::string& from);

  void HandleRequest(Request&& r, const std::string& from);
  void HandleDecision(Decision&& d, const std::string& from);

  void Propose();

  // Executes the given command and sends a reply to the client.
  void Execute(const Command& command);

  // The address of the leader.
  std::string leader_;

  int64_t slot_in_;
  int64_t slot_out_;
  std::queue<Command> requests_;
  std::unordered_map<int, Command> proposals_;
  // Only the leader stores the decided command. Followers will know a command
  // has been decided, but will not know what the command was.
  std::unordered_map<int, std::optional<Command>> decisions_;

  kvstore::Application app_;
};

}  // namespace paxos
}  // namespace process

#endif  // INCLUDE_PAXOS_REPLICA_HPP_
