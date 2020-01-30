// Copyright 2019 Lukas Joswiak

#include "process/paxos/leader.hpp"

#include <functional>
#include <thread>

#include "spdlog/spdlog.h"
#include "server/servers.hpp"

namespace process {
namespace paxos {

Leader::Leader(
    common::SharedQueue<Message>& message_queue,
    common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
        dispatch_queue,
    std::string& address)
    : PaxosProcess(message_queue, dispatch_queue),
      scout_id_(0),
      address_(address),
      active_(false) {
  ballot_number_.set_number(0);
  ballot_number_.set_address(address);
  logger_ = spdlog::get("leader");
}

void Leader::Run() {
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
  } else if (message.type() == Message_MessageType_STATUS) {
    Status s;
    message.message().UnpackTo(&s);
    HandleStatus(std::move(s), message.from());
  } else if (message.type() == Message_MessageType_LEADER_CHANGE) {
    LeaderChange l;
    message.message().UnpackTo(&l);
    HandleLeaderChange(std::move(l), message.from());
  }
}

void Leader::HandleStatus(Status&& s, const std::string& from) {
  logger_->debug("received status message");

  std::string principal = PrincipalServer(s.live());
  if (!IsLeader() && principal == address_ &&
      s.live_size() > kServers.size() / 2) {
    // Clean up state from previous scout.
    scouts_.erase(scout_id_);
    scout_message_queue_.erase(scout_id_);

    // This is the principal server among the alive servers. Increase ballot
    // number to be above current leader and attempt to become leader.
    ballot_number_.set_number(leader_ballot_number_.number() + 1);
    scout_message_queue_[scout_id_] =
        std::make_shared<common::SharedQueue<Message>>();
    scouts_.emplace(scout_id_, paxos::Scout(*scout_message_queue_[scout_id_],
                                            dispatch_queue_, address_,
                                            ballot_number_));
    // This syntax is necessary to refer to the correct overloaded Scout::Run
    // function.
    // See https://stackoverflow.com/a/14306975/986991.
    std::thread(static_cast<void (paxos::Scout::*)(int)>(&paxos::Scout::Run),
                &scouts_.at(scout_id_), scout_id_).detach();
    ++scout_id_;
  }
}

void Leader::HandleLeaderChange(LeaderChange&& l, const std::string& from) {
  logger_->debug("received leader change with ballot address {}",
                 l.leader_ballot_number().address());

  leader_ballot_number_ = l.leader_ballot_number();
  if (active_ && address_ != leader_ballot_number_.address()) {
    logger_->info("{} demoted", address_);
    active_ = false;
  }
}

void Leader::HandleProposal(Proposal&& p, const std::string& from) {
  logger_->debug("received proposal from {}", from);

  int slot_number = p.slot_number();
  if (proposals_.find(slot_number) == proposals_.end()) {
    proposals_[slot_number] = p.command();
    // TODO: Forward to leader?
    if (IsLeader()) {
      assert(commander_message_queue_.find(slot_number) ==
          commander_message_queue_.end());
      SpawnCommander(slot_number, p.command());
    }
  }
}

void Leader::HandleAdopted(Adopted&& a, const std::string& from) {
  logger_->debug("received Adopted from {}", from);

  if (CompareBallotNumbers(ballot_number_, a.ballot_number()) == 0) {
    logger_->info("{} promoted to leader", address_);
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

    leader_ballot_number_ = a.ballot_number();
    active_ = true;
  }
}

void Leader::HandlePreempted(Preempted&& p, const std::string& from) {
  logger_->debug("received Preempted from {}", from);
  logger_->info("{} demoted", address_);

  // When preempted, set leader equal to preempting server and don't try to
  // become leader again.
  leader_ballot_number_ = p.ballot_number();
  active_ = false;
}

void Leader::HandleP1B(Message&& m, const std::string& from) {
  // Deliver the message by adding it to the correct scout message queue.
  P1B p;
  m.message().UnpackTo(&p);
  auto scout_id = p.scout_id();

  if (scout_message_queue_.find(scout_id) != scout_message_queue_.end()) {
    auto scout_queue = scout_message_queue_.at(p.scout_id());
    scout_queue->push(m);
  }
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

std::string Leader::PrincipalServer(
    const google::protobuf::RepeatedPtrField<std::string>& servers) {
  std::string principal;
  for (const auto& server : servers) {
    if (server > principal) {
      principal = server;
    }
  }
  return principal;
}

void Leader::SpawnCommander(int slot_number, Command command) {
  logger_->trace("spawning commander for slot {}", slot_number);

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

bool Leader::IsLeader() {
  return active_ && address_ == leader_ballot_number_.address();
}

}  // namespace paxos
}  // namespace process
