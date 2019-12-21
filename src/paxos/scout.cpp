// Copyright 2019 Lukas Joswiak

#include "paxos/scout.hpp"

#include <iostream>

namespace paxos {

Scout::Scout(
    common::SharedQueue<Message>& message_queue,
    common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
        dispatch_queue,
    BallotNumber& ballot_number)
    : Process(message_queue, dispatch_queue),
      ballot_number_(ballot_number) {
  P1A p;    
  p.set_allocated_ballot_number(new BallotNumber(ballot_number));

  Message m;
  m.set_type(Message_MessageType_P1A);
  m.mutable_message()->PackFrom(p);

  dispatch_queue_.push(std::make_pair(std::nullopt, m));
}

void Scout::Handle(Message&& message) {
  if (message.type() == Message_MessageType_P1B) {
    P1B p;
    message.message().UnpackTo(&p);
    HandleP1B(std::move(p), message.from());
  }
}

void Scout::HandleP1B(P1B&& p, const std::string& from) {
  std::cout << "Received P1B from " << from << std::endl;
}

}  // namespace paxos
