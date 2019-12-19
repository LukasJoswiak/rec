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
  } else if (message.type() == Message_MessageType_PROPOSAL) {
    Proposal p;
    message.message().UnpackTo(&p);
    HandleProposal(std::move(p), message.from());
  }
}

void Replica::HandleRequest(Request&& r, const std::string& from) {
  std::cout << "Received Request from " << from << std::endl;

  requests_.push(r);

  Propose();
}

void Replica::HandleProposal(Proposal&& r, const std::string& from) {
  std::cout << "Received Proposal from " << from << std::endl;
}

void Replica::Propose() {
  auto request = requests_.front();
  requests_.pop();

  // Take ownership of the command object.
  auto command = request.release_command();

  Proposal proposal;
  proposal.set_slot_number(slot_in_++);
  proposal.set_allocated_command(command);

  Message m;
  m.set_type(Message_MessageType_PROPOSAL);
  m.mutable_message()->PackFrom(proposal);

  dispatch_queue_.push(std::make_pair(std::nullopt, m));
}

}  // namespace paxos
