#ifndef INCLUDE_PAXOS_LEADER_HPP_
#define INCLUDE_PAXOS_LEADER_HPP_

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "process/paxos/commander.hpp"
#include "process/paxos/paxos_process.hpp"
#include "process/paxos/scout.hpp"
#include "proto/internal.pb.h"
#include "proto/messages.pb.h"

namespace process {
namespace paxos {

class Leader : public PaxosProcess {
 public:
  Leader(
      common::SharedQueue<Message>& message_queue,
      common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
          dispatch_queue,
      std::string& address);

  void Run() override;

  void Handle(Message&& message) override;

 private:
  // Internal message handlers.
  void HandleStatus(Status&& s, const std::string& from);
  void HandleLeaderChange(LeaderChange&& l, const std::string& from);

  void HandleProposal(Proposal&& p, const std::string& from);
  void HandleAdopted(Adopted&& a, const std::string& from);
  void HandlePreempted(Preempted&& p, const std::string& from);

  // Handlers used to pass messages on to scout or commander.
  void HandleP1B(Message&& m, const std::string& from);
  void HandleP2B(Message&& m, const std::string& from);

  // Learn about decisions so commander state can be cleaned up.
  void HandleDecision(Decision&& d, const std::string& from);

  // Spawns and runs a scout on a thread with the given ballot number. Cleans up
  // any state associated with the previous scout.
  void SpawnScout(int ballot_number);

  // Spawns and runs a commander on a thread for the given slot number and
  // command.
  void SpawnCommander(int slot_number, Command command);

  // Given a list of servers from a protobuf message, returns the principal
  // server. The principal server is the server with the alphabetically maximum
  // address.
  //
  // For example:
  //   * servers = ["server1", "server2", "server3"]
  //   * returns "server3"
  //
  // Use this function to identify whether the current server should attempt
  // to become leader of the Paxos cluster.
  std::string PrincipalServer(
      const google::protobuf::RepeatedPtrField<std::string>& servers);

  // Returns true if this server is the leader.
  bool IsLeader();

  // Given a list of PValues received from acceptors, determine if any data
  // values are recoverable. If so, recover the value for the slot and add the
  // command to the list of commands to be proposed. This function should be
  // called by the leader after it has been elected and wants to recover data
  // from erasure coded chunks on other servers.
  void RecoverValues(
      const google::protobuf::RepeatedPtrField<PValue>& accepted_pvalues);

  // Since scouts are spawned on threads, it is possible to have multiple active
  // scouts at once. Use maps to keep track of a separate SharedQueue per scout.
  // When a new scout is spawned, old scouts will be killed as soon as possible.
  int scout_id_;
  std::unordered_map<int, paxos::Scout> scouts_;
  std::unordered_map<int, std::shared_ptr<common::SharedQueue<Message>>>
      scout_message_queue_;

  // Track each spawned commander, along with a SharedQueue for message passing
  // to that commander. Commanders are always spawned for a specific slot
  // number, and multiple commanders may be running at once.
  std::unordered_map<int, std::shared_ptr<paxos::Commander>> commanders_;
  std::unordered_map<int, std::shared_ptr<common::SharedQueue<Message>>>
      commander_message_queue_;

  BallotNumber ballot_number_;
  // TODO: Might be able to get rid of leader_ballot_number_ if servers retry
  // leader election if it fails (as long as it is still the principal)..
  // Ballot of the server this server thinks is the leader.
  BallotNumber leader_ballot_number_;
  bool active_;

  // Map of slot number -> Command proposed for the slot.
  std::unordered_map<int, Command> proposals_;
};

}  // namespace paxos
}  // namespace process

#endif  // INCLUDE_PAXOS_LEADER_HPP_
