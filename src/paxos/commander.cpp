// Copyright 2019 Lukas Joswiak

#include "paxos/commander.hpp"

#include <iostream>

namespace paxos {

Commander::Commander(
    common::SharedQueue<Message>& message_queue,
    common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
        dispatch_queue,
    BallotNumber ballot_number, int slot_number, Command command)
    : Process(message_queue, dispatch_queue),
      ballot_number_(ballot_number),
      slot_number_(slot_number),
      command_(command) {}

void Commander::Run() {
  P2A p;    
  p.set_allocated_ballot_number(new BallotNumber(ballot_number_));
  p.set_slot_number(slot_number_);
  p.set_allocated_command(new Command(command_));

  Message m;
  m.set_type(Message_MessageType_P2A);
  m.mutable_message()->PackFrom(p);

  dispatch_queue_.push(std::make_pair(std::nullopt, m));

  // Begin listening for incoming messages.
  Process::Run();
}

void Commander::Handle(Message&& message) {
  if (message.type() == Message_MessageType_P2B) {
    P2B p;
    message.message().UnpackTo(&p);
    HandleP2B(std::move(p), message.from());
  }
}

void Commander::HandleP2B(P2B&& p, const std::string& from) {
  std::cout << "Commander received P2B from " << from << std::endl;
  if (CompareBallotNumbers(ballot_number_, p.ballot_number()) == 0) {
    received_from_.insert(from);
    
    // Hardcoding quorum size of 2 for now.
    // TODO: modify quorum size to be configurable.
    if (received_from_.size() >= 2) {
      Decision d;
      d.set_slot_number(slot_number_);
      d.set_allocated_command(new Command(command_));

      Message m;
      m.set_type(Message_MessageType_DECISION);
      m.mutable_message()->PackFrom(d);

      dispatch_queue_.push(std::make_pair(std::nullopt, m));
      // TODO: kill thread 
    }
  } else {
    // TODO: send preempted message
    std::cout << "Commander preempted" << std::endl;
    // TODO: kill thread 
  }
}

}  // namespace paxos
