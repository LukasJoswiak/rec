// Copyright 2019 Lukas Joswiak

#include "paxos/leader.hpp"

#include <iostream>
#include <thread>

namespace paxos {

Leader::Leader(
    common::SharedQueue<Message>& message_queue,
    common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
        dispatch_queue,
    std::string& address)
    : Process(message_queue, dispatch_queue),
      address_(address),
      active_(false) {
  ballot_number_.set_number(0);
  ballot_number_.set_address(address);
}

void Leader::Run() {
  // Spawn a scout to start leader election.
  scout_ = std::make_shared<paxos::Scout>(scout_message_queue_, dispatch_queue_,
                                          address_, ballot_number_);
  std::thread(&paxos::Scout::Run, scout_).detach();

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
  }
}

void Leader::HandleProposal(Proposal&& p, const std::string& from) {
  std::cout << "Leader received proposal from " << from << std::endl;
  int slot_number = p.slot_number();
  if (proposals_.find(slot_number) == proposals_.end()) {
    proposals_[slot_number] = p.command();
    if (active_) {
      assert(commander_message_queue_.find(slot_number) ==
          commander_message_queue_.end());
      SpawnCommander(slot_number, p.command());
    }
  }
}

void Leader::HandleAdopted(Adopted&& a, const std::string& from) {
  std::cout << "Received adopted from " << from << std::endl;
  if (CompareBallotNumbers(ballot_number_, a.ballot_number()) == 0) {
    // Map of slot number -> ballot, used to determine the command to propose
    // for each slot.
    std::unordered_map<int, BallotNumber> max_ballots;
    for (int i = 0; i < a.accepted_size(); ++i) {
      PValue pvalue = a.accepted(i);
      if (max_ballots.find(pvalue.slot_number()) == max_ballots.end() ||
          CompareBallotNumbers(max_ballots.at(pvalue.slot_number()),
                               pvalue.ballot_number()) > 0) {
        max_ballots[pvalue.slot_number()] = pvalue.ballot_number();
        proposals_[pvalue.slot_number()] = pvalue.command();
      }
    }

    // Propose all commands that have been accepted by other servers.
    for (const auto& it : proposals_) {
      int slot_number = it.first;
      Command command = it.second;

      SpawnCommander(slot_number, command);
    }

    active_ = true;
  }
}

void Leader::HandlePreempted(Preempted&& p, const std::string& from) {
  std::cout << "Received preempted from " << from << std::endl;
  if (CompareBallotNumbers(ballot_number_, p.ballot_number()) > 0) {
    ballot_number_.set_number(ballot_number_.number() + 1);

    // Spawn a new scout to begin leader election with updated ballot.
    scout_ = std::make_shared<paxos::Scout>(scout_message_queue_,
                                            dispatch_queue_, address_,
                                            ballot_number_);
    std::thread(&paxos::Scout::Run, scout_).detach();
  }
  active_ = false;
}

void Leader::HandleP1B(Message&& m, const std::string& from) {
  scout_message_queue_.push(m);
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

void Leader::SpawnCommander(int slot_number, Command command) {
  std::cout << "Spawning commander for slot number " << slot_number
            << std::endl;
  // Create a SharedQueue for the commander to allow passing of messages to it.
  commander_message_queue_[slot_number] =
      std::make_shared<common::SharedQueue<Message>>();

  // Create a commander and run it on its own thread.
  commanders_.emplace(slot_number,
      Commander(*commander_message_queue_[slot_number], dispatch_queue_,
                address_, ballot_number_, slot_number, command));
  std::thread(&paxos::Commander::Run, &commanders_.at(slot_number))
      .detach();
}

}  // namespace paxos
