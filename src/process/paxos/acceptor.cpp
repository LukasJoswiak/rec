// Copyright 2019 Lukas Joswiak

#include "process/paxos/acceptor.hpp"

namespace process {
namespace paxos {

Acceptor::Acceptor(
    common::SharedQueue<Message>& message_queue,
    common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
        dispatch_queue)
    : Process(message_queue, dispatch_queue) {
  logger_ = spdlog::get("acceptor");
}

void Acceptor::Handle(Message&& message) {
  if (message.type() == Message_MessageType_P1A) {
    P1A p;
    message.message().UnpackTo(&p);
    HandleP1A(std::move(p), message.from());
  } else if (message.type() == Message_MessageType_P2A) {
    P2A p;
    message.message().UnpackTo(&p);
    HandleP2A(std::move(p), message.from());
  }
}

void Acceptor::HandleP1A(P1A&& p, const std::string& from) {
  logger_->debug("received P1A from {}", from);
  if (CompareBallotNumbers(ballot_number_, p.ballot_number()) > 0) {
    ballot_number_ = p.ballot_number();
  }

  P1B p1b;
  p1b.set_allocated_ballot_number(new BallotNumber(ballot_number_));
  p1b.clear_accepted();
  for (const auto& pvalue : accepted_) {
    PValue* new_pvalue = p1b.add_accepted();
    new_pvalue->set_allocated_ballot_number(
        new BallotNumber(pvalue.ballot_number()));
    new_pvalue->set_slot_number(pvalue.slot_number());
    new_pvalue->set_allocated_command(new Command(pvalue.command()));
  }

  Message m;
  m.set_type(Message_MessageType_P1B);
  m.mutable_message()->PackFrom(p1b);

  logger_->debug("sending P1B to {}", from);
  dispatch_queue_.push(std::make_pair(from, m));
}

void Acceptor::HandleP2A(P2A&& p, const std::string& from) {
  logger_->debug("received P2A from {}", from);
  if (CompareBallotNumbers(ballot_number_, p.ballot_number()) == 0) {
    PValue pvalue;
    pvalue.set_allocated_ballot_number(new BallotNumber(p.ballot_number()));
    pvalue.set_slot_number(p.slot_number());
    pvalue.set_allocated_command(new Command(p.command()));

    accepted_.emplace(pvalue);
  }

  P2B p2b;
  p2b.set_allocated_ballot_number(new BallotNumber(ballot_number_));
  p2b.set_slot_number(p.slot_number());

  Message m;
  m.set_type(Message_MessageType_P2B);
  m.mutable_message()->PackFrom(p2b);

  logger_->debug("sending P2B to {}", from);
  dispatch_queue_.push(std::make_pair(from, m));
}

}  // namespace paxos
}  // namespace process
#include "process/process.hpp"
