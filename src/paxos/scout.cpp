// Copyright 2019 Lukas Joswiak

#include "paxos/scout.hpp"

#include <iostream>

namespace paxos {

Scout::Scout(
    common::SharedQueue<Message>& message_queue,
    common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
        dispatch_queue,
    std::string& leader, BallotNumber& ballot_number)
    : Process(message_queue, dispatch_queue),
      leader_(leader),
      ballot_number_(ballot_number) {}

Scout::~Scout() {
  exit_ = true;
}

void Scout::Run() {
  P1A p;
  p.set_allocated_ballot_number(new BallotNumber(ballot_number_));

  Message m;
  m.set_type(Message_MessageType_P1A);
  m.mutable_message()->PackFrom(p);

  dispatch_queue_.push(std::make_pair(std::nullopt, m));

  // Begin listening for incoming messages.
  Process::Run();
}

void Scout::Handle(Message&& message) {
  if (message.type() == Message_MessageType_P1B) {
    P1B p;
    message.message().UnpackTo(&p);
    HandleP1B(std::move(p), message.from());
  }
}

void Scout::HandleP1B(P1B&& p, const std::string& from) {
  std::cout << "Scout received P1B from " << from << std::endl;
  if (CompareBallotNumbers(ballot_number_, p.ballot_number()) == 0) {
    received_from_.insert(from);
    // TODO: Check for duplicates.
    for (int i = 0; i < p.accepted_size(); ++i) {
      pvalues_.push_back(p.accepted(i));
    }

    // TODO: don't hardcode quorum size.
    if (received_from_.size() >= 2) {
      Adopted a;
      a.set_allocated_ballot_number(new BallotNumber(ballot_number_));

      for (const auto& pvalue : pvalues_) {
        PValue* new_pvalue = a.add_accepted();
        new_pvalue->set_allocated_ballot_number(
            new BallotNumber(pvalue.ballot_number()));
        new_pvalue->set_slot_number(pvalue.slot_number());
        new_pvalue->set_allocated_command(new Command(pvalue.command()));
      }

      Message m;
      m.set_type(Message_MessageType_ADOPTED);
      m.mutable_message()->PackFrom(a);

      dispatch_queue_.push(std::make_pair(leader_, m));
      exit_ = true;
    }
  } else {
    std::cout << "Scout preempted" << std::endl;
    exit_ = true;
  }
}

}  // namespace paxos
