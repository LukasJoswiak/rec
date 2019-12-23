// Copyright 2019 Lukas Joswiak

#include "paxos/replica.hpp"

#include <optional>

namespace paxos {

Replica::Replica(
    common::SharedQueue<Message>& message_queue,
    common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
        dispatch_queue)
    : Process(message_queue, dispatch_queue),
      slot_in_(1),
      slot_out_(1) {}

void Replica::Handle(Message&& message) {
  if (message.type() == Message_MessageType_REQUEST) {
    Request r;
    message.message().UnpackTo(&r);
    HandleRequest(std::move(r), message.from());
  } else if (message.type() == Message_MessageType_DECISION) {
    Decision d;
    message.message().UnpackTo(&d);
    HandleDecision(std::move(d), message.from());
  }
}

void Replica::HandleRequest(Request&& r, const std::string& from) {
  requests_.push(r);
  Propose();
}

void Replica::HandleDecision(Decision&& d, const std::string& from) {
  int slot_number = d.slot_number();

  decisions_[slot_number] = d.command();
  while (decisions_.find(slot_out_) != decisions_.end()) {
    if (proposals_.find(slot_out_) != proposals_.end()) {
      // If this replica proposed a different command than the one that was
      // chosen, add the command to the end of the list of proposals so it will
      // be retried.
      if (!CommandsEqual(proposals_[slot_out_].command(),
                         decisions_[slot_out_])) {
        requests_.push(proposals_[slot_out_]);
      }

      proposals_.erase(slot_out_);
    }

    std::cout << "Executing command for slot " << slot_out_ << std::endl;
    ++slot_out_;
  }
}

void Replica::Propose() {
  while (!requests_.empty()) {
    if (decisions_.find(slot_in_) == decisions_.end()) {
      auto request = requests_.front();
      requests_.pop();

      proposals_[slot_in_] = request;

      // Take ownership of the command object.
      auto command = request.release_command();

      Proposal proposal;
      proposal.set_slot_number(slot_in_);
      proposal.set_allocated_command(command);

      Message m;
      m.set_type(Message_MessageType_PROPOSAL);
      m.mutable_message()->PackFrom(proposal);

      // Send proposal to all servers for now.
      // TODO: only send proposal to leaders.
      std::cout << "Proposing command for slot " << slot_in_ << std::endl;
      dispatch_queue_.push(std::make_pair(std::nullopt, m));

      ++slot_in_;
    }
  }
}

bool Replica::CommandsEqual(const Command& c1, const Command& c2) {
  return c1.sequence_number() == c2.sequence_number() &&
      c1.operation() == c2.operation();
}

}  // namespace paxos
