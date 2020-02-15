#include "process/paxos/leader.hpp"

#include <functional>
#include <thread>
#include <unordered_map>

#include "process/paxos/compare.hpp"
#include "spdlog/spdlog.h"
#include "server/code.hpp"
#include "server/servers.hpp"

namespace process {
namespace paxos {

Leader::Leader(
    common::SharedQueue<Message>& message_queue,
    common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
        dispatch_queue,
    std::string& address)
    : PaxosProcess(message_queue, dispatch_queue, address),
      scout_id_(0),
      active_(false) {
  ballot_number_.set_number(0);
  ballot_number_.set_address(address);
  logger_ = spdlog::get("leader");
}

void Leader::Run() {
  // Begin listening for incoming messages.
  Process::Run();
}

void Leader::Handle(Message&& message) {
  if (message.type() == Message_MessageType_PROPOSAL) {
    Proposal p;
    message.message().UnpackTo(&p);
    HandleProposal(std::move(p), message.from());
  } else if (message.type() == Message_MessageType_ADOPTED) {
    Adopted a;
    message.message().UnpackTo(&a);
    HandleAdopted(std::move(a), message.from());
  } else if (message.type() == Message_MessageType_PREEMPTED) {
    Preempted p;
    message.message().UnpackTo(&p);
    HandlePreempted(std::move(p), message.from());
  } else if (message.type() == Message_MessageType_P1B) {
    HandleP1B(std::move(message), message.from());
  } else if (message.type() == Message_MessageType_P2B) {
    HandleP2B(std::move(message), message.from());
  } else if (message.type() == Message_MessageType_DECISION) {
    Decision d;
    message.message().UnpackTo(&d);
    HandleDecision(std::move(d), message.from());
  } else if (message.type() == Message_MessageType_STATUS) {
    Status s;
    message.message().UnpackTo(&s);
    HandleStatus(std::move(s), message.from());
  } else if (message.type() == Message_MessageType_LEADER_CHANGE) {
    LeaderChange l;
    message.message().UnpackTo(&l);
    HandleLeaderChange(std::move(l), message.from());
  }
}

void Leader::HandleStatus(Status&& s, const std::string& from) {
  logger_->debug("received status message");

  std::string principal = PrincipalServer(s.live());
  if (!IsLeader() && principal == address_ &&
      s.live_size() >= kQuorum) {
    // This is the principal server among the alive servers. Increase ballot
    // number to be above current leader and attempt to become leader.
    SpawnScout(leader_ballot_number_.number() + 1);
  }
}

void Leader::HandleLeaderChange(LeaderChange&& l, const std::string& from) {
  logger_->debug("received leader change with ballot address {}",
                 l.leader_ballot_number().address());

  leader_ballot_number_ = l.leader_ballot_number();
  if (active_ && address_ != leader_ballot_number_.address()) {
    logger_->info("{} demoted", address_);
    active_ = false;
  }
}

void Leader::HandleProposal(Proposal&& p, const std::string& from) {
  logger_->debug("received proposal from {}", from);

  if (!IsLeader()) {
    return;
  }

  int slot_number = p.slot_number();
  if (proposals_.find(slot_number) == proposals_.end()) {
    proposals_[slot_number] = p.command();

    assert(commander_message_queue_.find(slot_number) ==
        commander_message_queue_.end());
    SpawnCommander(slot_number, p.command());
  }
}

void Leader::HandleAdopted(Adopted&& a, const std::string& from) {
  logger_->debug("received Adopted from {}", from);

  if (CompareBallotNumbers(ballot_number_, a.ballot_number()) == 0) {
    logger_->info("{} promoted to leader", address_);

    // Recover any values and add the command to proposals_ to be reproposed.
    RecoverValues(a.accepted());

    // Propose all commands that have been accepted by other servers. Must have
    // received enough responses such that this server was able to reconstruct
    // the value from the coded chunks.
    for (const auto& it : proposals_) {
      int slot_number = it.first;
      Command command = it.second;

      // Notify the replica of the proposed command for each slot.
      ReconstructedProposal p;
      p.set_slot_number(slot_number);
      p.set_allocated_command(new Command(command));

      Message m;
      m.set_type(Message_MessageType_RECONSTRUCTED_PROPOSAL);
      m.mutable_message()->PackFrom(p);

      dispatch_queue_.push(std::make_pair(address_, m));

      // Spawn commander to reach consensus on reconstructed command.
      SpawnCommander(slot_number, command);
    }
    // TODO: Send no-ops for slots without enough data to reconstruct.

    leader_ballot_number_ = a.ballot_number();
    active_ = true;
  }
}

void Leader::HandlePreempted(Preempted&& p, const std::string& from) {
  logger_->debug("received Preempted from {}", from);
  logger_->info("{} demoted", address_);

  if (p.ballot_number().address() < address_) {
    // This server has a higher address than the ballot from the server we were
    // preempted by, so try to become leader again with a higher ballot.
    SpawnScout(std::max(ballot_number_.number(),
                        p.ballot_number().number()) + 1);
  } else {
    // Preempted by a server with a higher address. Set leader equal to address
    // in preempting ballot and don't try to become leader again.
    leader_ballot_number_ = p.ballot_number();
    active_ = false;
  }
}

void Leader::HandleP1B(Message&& m, const std::string& from) {
  // Deliver the message by adding it to the correct scout message queue.
  P1B p;
  m.message().UnpackTo(&p);
  auto scout_id = p.scout_id();

  if (scout_message_queue_.find(scout_id) != scout_message_queue_.end()) {
    auto scout_queue = scout_message_queue_.at(p.scout_id());
    scout_queue->push(m);
  }
}

void Leader::HandleP2B(Message&& m, const std::string& from) {
  // Deliver the message by adding it to the correct commander message queue.
  P2B p;
  m.message().UnpackTo(&p);
  int slot_number = p.slot_number();
  assert(commander_message_queue_.find(slot_number) !=
      commander_message_queue_.end());

  auto commander_queue = commander_message_queue_.at(slot_number);
  commander_queue->push(m);
}

void Leader::HandleDecision(Decision&& d, const std::string& from) {
  // Clean up state for commander where value has been decided.
  commanders_.erase(d.slot_number());
  commander_message_queue_.erase(d.slot_number());
}

void Leader::SpawnScout(int ballot_number) {
  logger_->trace("spawning scout with ballot number {}", ballot_number);

  // Clean up state from previous scout.
  scouts_.erase(scout_id_);
  scout_message_queue_.erase(scout_id_);

  ballot_number_.set_number(ballot_number);

  // Create a new shared queue to pass messages to the scout.
  scout_message_queue_[scout_id_] =
      std::make_shared<common::SharedQueue<Message>>();
  scouts_.emplace(scout_id_, paxos::Scout(*scout_message_queue_[scout_id_],
                                          dispatch_queue_, address_,
                                          ballot_number_));
  // This syntax is necessary to refer to the correct overloaded Scout::Run
  // function.
  // See https://stackoverflow.com/a/14306975/986991.
  std::thread(static_cast<void (paxos::Scout::*)(int)>(&paxos::Scout::Run),
              &scouts_.at(scout_id_), scout_id_).detach();
  ++scout_id_;
}

void Leader::SpawnCommander(int slot_number, Command command) {
  logger_->trace("spawning commander for slot {}", slot_number);

  // Create a SharedQueue for the commander to allow passing of messages to it.
  commander_message_queue_[slot_number] =
      std::make_shared<common::SharedQueue<Message>>();

  // If a commander exists for this slot, remove it so a new commander can be
  // created.
  if (commanders_.find(slot_number) != commanders_.end()) {
    commanders_.erase(slot_number);
  }

  // Create a commander and run it on its own thread.
  auto pair = commanders_.emplace(slot_number,
      Commander(*commander_message_queue_[slot_number], dispatch_queue_,
                address_, ballot_number_, slot_number, command));
  // Make sure the new commander was successfully inserted.
  assert(std::get<1>(pair) == true);
  std::thread(&paxos::Commander::Run, &commanders_.at(slot_number))
      .detach();
}

std::string Leader::PrincipalServer(
    const google::protobuf::RepeatedPtrField<std::string>& servers) {
  std::string principal;
  for (const auto& server : servers) {
    if (server > principal) {
      principal = server;
    }
  }
  return principal;
}

bool Leader::IsLeader() {
  return active_ && address_ == leader_ballot_number_.address();
}

void Leader::RecoverValues(
    const google::protobuf::RepeatedPtrField<PValue>& accepted_pvalues) {
  // Map of slot number -> map of ballot number -> vector of pvalues received
  // from acceptors for the slot and ballot.
  std::unordered_map<int,
      std::unordered_map<BallotNumber, std::vector<PValue>, BallotHash,
                         BallotEqualTo>> pvalues;

  // TODO: Don't recover value if it has already been learned.

  // Need to determine if this server received enough responses for each slot
  // to be able to reconstruct the original data value, in which case it can
  // be reproposed. Otherwise, a no-op will be proposed for the slot.
  for (const auto& pvalue : accepted_pvalues) {
    int slot_number = pvalue.slot_number();
    const BallotNumber& ballot_number = pvalue.ballot_number();
    if (pvalues.find(slot_number) == pvalues.end()) {
      pvalues[slot_number] =
          std::unordered_map<BallotNumber, std::vector<PValue>, BallotHash,
                             BallotEqualTo>();
    }

    auto& slot_pvalues = pvalues[slot_number];
    if (slot_pvalues.find(ballot_number) == slot_pvalues.end()) {
      slot_pvalues[ballot_number] = std::vector<PValue>();
    }

    auto& slot_ballot_pvalues = slot_pvalues[ballot_number];
    slot_ballot_pvalues.push_back(pvalue);

    if (slot_ballot_pvalues.size() == Code::kOriginalBlocks) {
      // Received enough responses to regenerate data for this slot.
      logger_->trace("regenerating command for slot {}", slot_number);

      int block_size = slot_ballot_pvalues[0].command().value().size();

      // Recover original value from original and redundant shares received
      // from acceptors.
      cm256_block blocks[Code::kOriginalBlocks];
      int i = 0;
      for (const auto& pvalue : slot_ballot_pvalues) {
        // Make sure size of each block is the same.
        assert(pvalue.command().value().size() == block_size);

        blocks[i].Index = pvalue.command().block_index();
        blocks[i].Block = (unsigned char*) pvalue.command().value().data();
        ++i;
      }
      bool success = Code::Decode(blocks, block_size);
      assert(success == true);

      // TODO: Might need to store this on the heap if values become large
      // Create recovered string of correct size, then fill it with data from
      // each block. Blocks might be returned out of order.
      std::string recovered_value(4 * Code::kOriginalBlocks, '0');
      for (int i = 0; i < Code::kOriginalBlocks; ++i) {
        int index = blocks[i].Index;
        recovered_value.replace(index * block_size, block_size, std::string((char*) blocks[i].Block));
      }

      // TODO: Might need to store this on the heap
      Command command = Command(slot_ballot_pvalues[0].command());
      command.set_value(recovered_value);
      command.clear_block_index();
      proposals_[slot_number] = command;
    }
  }
}

}  // namespace paxos
}  // namespace process
