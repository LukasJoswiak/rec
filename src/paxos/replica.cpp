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
  }
}

void Replica::HandleRequest(Request&& r, const std::string& from) {
  requests_.push(r);
  Propose();
}

void Replica::Propose() {
  while (!requests_.empty()) {
    if (decisions_.find(slot_in_) == decisions_.end()) {
      auto request = requests_.front();
      requests_.pop();

      // Take ownership of the command object.
      auto command = request.release_command();

      Proposal proposal;
      proposal.set_slot_number(slot_in_);
      proposal.set_allocated_command(command);
      proposals_[slot_in_++] = proposal;

      Message m;
      m.set_type(Message_MessageType_PROPOSAL);
      m.mutable_message()->PackFrom(proposal);

      // Send proposal to all servers for now.
      // TODO: only send proposal to leaders.
      dispatch_queue_.push(std::make_pair(std::nullopt, m));
    }
  }
}

}  // namespace paxos
