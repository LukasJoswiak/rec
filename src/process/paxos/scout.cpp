#include "process/paxos/scout.hpp"

#include "server/servers.hpp"

namespace process {
namespace paxos {

Scout::Scout(
    common::SharedQueue<Message>& message_queue,
    common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
        dispatch_queue,
    std::string& leader, BallotNumber& ballot_number)
    : PaxosProcess(message_queue, dispatch_queue),
      leader_(leader),
      ballot_number_(ballot_number) {
  logger_ = spdlog::get("scout");
}

Scout::~Scout() {
  logger_->trace("exiting");
  exit_ = true;
}

void Scout::Run(int scout_id) {
  P1A p;
  p.set_scout_id(scout_id);
  p.set_allocated_ballot_number(new BallotNumber(ballot_number_));

  Message m;
  m.set_type(Message_MessageType_P1A);
  m.mutable_message()->PackFrom(p);

  logger_->debug("broadcasting P1As");
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
  logger_->debug("received P1B from {}", from);

  if (CompareBallotNumbers(ballot_number_, p.ballot_number()) == 0) {
    received_from_.insert(from);
    logger_->trace("received {}/{} responses", received_from_.size(),
                   kServers.size());
    for (int i = 0; i < p.accepted_size(); ++i) {
      pvalues_.insert(p.accepted(i));
    }

    if (received_from_.size() > kServers.size() / 2) {
      // Received promises from a majority of acceptors to not accept requests
      // with a lower ballot number. Can now promote this server to be leader.
      Adopted a;
      a.set_allocated_ballot_number(new BallotNumber(ballot_number_));

      // If any acceptors have accepted a value for the current ballot, we must
      // propose the associated command.
      // TODO: Update to work with partial, erasure-coded values.
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

      logger_->debug("sending Adopted to leader {}", leader_);
      dispatch_queue_.push(std::make_pair(leader_, m));
      exit_ = true;
    }
  } else {
    Preempted p;
    p.set_allocated_ballot_number(new BallotNumber(p.ballot_number()));

    Message m;
    m.set_type(Message_MessageType_PREEMPTED);
    m.mutable_message()->PackFrom(p);

    logger_->debug("sending Preempted to leader {}", leader_);
    dispatch_queue_.push(std::make_pair(leader_, m));
    exit_ = true;
  }
}

}  // namespace paxos
}  // namespace process
