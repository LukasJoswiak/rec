// Copyright 2019 Lukas Joswiak

#include "process/paxos/commander.hpp"

#include "cm256/cm256.h"
#include "server/servers.hpp"
#include "server/code.hpp"

namespace process {
namespace paxos {

Commander::Commander(
    common::SharedQueue<Message>& message_queue,
    common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
        dispatch_queue,
    std::string& leader, BallotNumber& ballot_number, int slot_number,
    Command& command)
    : Process(message_queue, dispatch_queue),
      leader_(leader),
      ballot_number_(ballot_number),
      slot_number_(slot_number),
      command_(command) {
  logger_= spdlog::get("commander");
}

Commander::~Commander() {
  logger_->trace("exiting for slot {}", slot_number_);
  exit_ = true;
}

void Commander::Run() {
  // Coded data shares.
  std::array<std::string, Code::kTotalBlocks> shares;

  // Operations with data should be split into chunks and split between servers.
  if (command_.operation() != Command_Operation_GET) {
    int block_size;
    cm256_block blocks[Code::kOriginalBlocks];
    uint8_t* recovery_blocks;
    if (!Code::Encode((unsigned int*) command_.value().data(),
                      command_.value().size(), blocks,
                      &recovery_blocks, &block_size)) {
      logger_->error("encoding error, exiting...");
      exit(1);
    }

    // Copy original blocks.
    for (int i = 0; i < Code::kOriginalBlocks; ++i) {
      shares[i] = std::string((char*) blocks[i].Block, block_size);
    }

    // Copy redundant blocks.
    for (int i = 0; i < Code::kRedundantBlocks; ++i) {
      shares[Code::kOriginalBlocks + i] = std::string((char*) recovery_blocks,
                                                      i * block_size,
                                                      block_size);
    }

    delete recovery_blocks;
  }

  logger_->trace("broadcasting P2As for slot {}", slot_number_);
  for (int i = 0; i < kServers.size(); ++i) {
    std::string server_name = std::get<0>(kServers[i]);

    std::string data = shares[i];
    auto command = new Command(command_);
    /*
    // TODO: Uncomment to send erasure coded chunks to acceptors.
    // Not all requests have associated data.
    if (data.size() > 0) {
      command->set_value(data);
      // TODO: set block index in command.
    }
    */

    P2A p;
    p.set_allocated_ballot_number(new BallotNumber(ballot_number_));
    p.set_slot_number(slot_number_);
    p.set_allocated_command(command);

    Message m;
    m.set_type(Message_MessageType_P2A);
    m.mutable_message()->PackFrom(p);

    dispatch_queue_.push(std::make_pair(server_name, m));
  }

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
  logger_->debug("received P2B from {} for slot {}", from, slot_number_);
  int compared_ballots = CompareBallotNumbers(ballot_number_, p.ballot_number());
  if (compared_ballots == 0) {
    received_from_.insert(from);

    if (received_from_.size() > kServers.size() / 2) {
      Decision d;
      d.set_slot_number(slot_number_);
      d.set_allocated_command(new Command(command_));

      Message m;
      m.set_type(Message_MessageType_DECISION);
      m.mutable_message()->PackFrom(d);

      logger_->debug("broadcasting Decision");
      dispatch_queue_.push(std::make_pair(std::nullopt, m));
      exit_ = true;
    }
  } else if (compared_ballots > 0) {
    // Only preempt if the message ballot is larger than this servers ballot,
    // which means another server has a higher ballot number and should be
    // leader.
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