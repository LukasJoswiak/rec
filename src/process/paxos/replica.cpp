#include "process/paxos/replica.hpp"

#include <optional>

#include "google/protobuf/util/message_differencer.h"

namespace process {
namespace paxos {

Replica::Replica(
    common::SharedQueue<Message>& message_queue,
    common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
        dispatch_queue,
    std::string& address)
    : PaxosProcess(message_queue, dispatch_queue, address),
      slot_in_(1),
      slot_out_(1) {
  logger_ = spdlog::get("replica");
}

void Replica::Handle(Message&& message) {
  if (message.type() == Message_MessageType_REQUEST) {
    Request r;
    message.message().UnpackTo(&r);
    HandleRequest(std::move(r), message.from());
  } else if (message.type() == Message_MessageType_DECISION) {
    Decision d;
    message.message().UnpackTo(&d);
    HandleDecision(std::move(d), message.from());
  } else if (message.type() == Message_MessageType_LEADER_CHANGE) {
    LeaderChange l;
    message.message().UnpackTo(&l);
    HandleLeaderChange(std::move(l), message.from());
  }
}

void Replica::HandleRequest(Request&& r, const std::string& from) {
  if (address_ == leader_) {
    // Propose command if this server thinks it is the leader.
    requests_.push(r.command());
    Propose();
  } else if (leader_.size() > 0) {
    // Forward request to leader if it is known.
    logger_->debug("forwarding request to leader {}", leader_);

    Message m;
    m.set_type(Message_MessageType_REQUEST);
    m.mutable_message()->PackFrom(r);

    dispatch_queue_.push(std::make_pair(leader_, m));
  }
}

void Replica::HandleDecision(Decision&& d, const std::string& from) {
  int slot_number = d.slot_number();
  logger_->debug("received Decision for slot {}", slot_number);

  if (address_ == leader_) {
    // Only the leader executes commands because only the leader stores the
    // entire command. Followers each store smaller erasure coded chunks.
    // decisions_[slot_number] = d.command();
    decisions_[slot_number] = proposals_[slot_number];
    while (decisions_.find(slot_out_) != decisions_.end()) {
      if (proposals_.find(slot_out_) != proposals_.end()) {
        proposals_.erase(slot_out_);
      }

      Execute(decisions_[slot_number]);
      ++slot_out_;
    }
  }
}

void Replica::HandleLeaderChange(LeaderChange&& l, const std::string& from) {
  logger_->debug("received leader change with ballot address {}",
                 l.leader_ballot_number().address());
  leader_ = l.leader_ballot_number().address();
}

void Replica::Propose() {
  // Propose should only be called on the leader.
  assert(leader_ == address_);

  while (!requests_.empty()) {
    if (decisions_.find(slot_in_) == decisions_.end()) {
      auto command = requests_.front();
      requests_.pop();

      if (app_.Executed(command)) {
        logger_->debug("already executed command, resending response");
        Execute(command);
        continue;
      }

      proposals_[slot_in_] = command;

      Proposal proposal;
      proposal.set_slot_number(slot_in_);
      proposal.set_allocated_command(new Command(command));

      Message m;
      m.set_type(Message_MessageType_PROPOSAL);
      m.mutable_message()->PackFrom(proposal);

      // Send proposal to leader thread.
      logger_->debug("proposing command for slot {}", slot_in_);
      dispatch_queue_.push(std::make_pair(address_, m));
    }

    ++slot_in_;
  }
}

void Replica::Execute(const Command& command) {
  logger_->info("executing command");
  if (auto response = app_.Execute(command)) {
    Message m;
    m.set_type(Message_MessageType_RESPONSE);
    m.mutable_message()->PackFrom(*response);

    logger_->info("sending response to {}", command.client());
    dispatch_queue_.push(std::make_pair(command.client(), m));
  } else {
    logger_->debug("failed to execute command");
  }
}

}  // namespace paxos
}  // namespace process
