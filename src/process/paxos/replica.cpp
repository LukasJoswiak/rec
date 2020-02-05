#include "process/paxos/replica.hpp"

#include <optional>

#include "google/protobuf/util/message_differencer.h"

namespace process {
namespace paxos {

Replica::Replica(
    common::SharedQueue<Message>& message_queue,
    common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
        dispatch_queue)
    : PaxosProcess(message_queue, dispatch_queue),
      slot_in_(1),
      slot_out_(1) {
  logger_ = spdlog::get("replica");
}

void Replica::Handle(Message&& message) {
  if (message.type() == Message_MessageType_REQUEST) {
    Request r;
    message.message().UnpackTo(&r);
    HandleRequest(std::move(r), message.from());
  } else if (message.type() == Message_MessageType_DECISION) {
    Decision d;
    message.message().UnpackTo(&d);
    HandleDecision(std::move(d), message.from());
  }
}

void Replica::HandleRequest(Request&& r, const std::string& from) {
  requests_.push(r.command());
  Propose();
}

void Replica::HandleDecision(Decision&& d, const std::string& from) {
  int slot_number = d.slot_number();
  logger_->debug("received Decision for slot {}", slot_number);

  decisions_[slot_number] = d.command();
  while (decisions_.find(slot_out_) != decisions_.end()) {
    if (proposals_.find(slot_out_) != proposals_.end()) {
      // If this replica proposed a different command than the one that was
      // chosen, add the command to the end of the list of proposals so it will
      // be retried.
      if (!google::protobuf::util::MessageDifferencer::Equals(
            proposals_[slot_out_],
            decisions_[slot_out_])) {
        requests_.push(proposals_[slot_out_]);
      }

      proposals_.erase(slot_out_);
    }

    Execute(d.command());
  }
}

void Replica::Propose() {
  while (!requests_.empty()) {
    if (decisions_.find(slot_in_) == decisions_.end()) {
      auto command = requests_.front();
      requests_.pop();

      proposals_[slot_in_] = command;

      Proposal proposal;
      proposal.set_slot_number(slot_in_);
      proposal.set_allocated_command(new Command(command));

      Message m;
      m.set_type(Message_MessageType_PROPOSAL);
      m.mutable_message()->PackFrom(proposal);

      // Send proposal to all servers for now.
      // TODO: only send proposal to leaders.
      logger_->debug("proposing command for slot {}", slot_in_);
      dispatch_queue_.push(std::make_pair(std::nullopt, m));

      ++slot_in_;
    }
  }
}

void Replica::Execute(const Command& command) {
  for (int i = 1; i < slot_out_; ++i) {
    if (google::protobuf::util::MessageDifferencer::Equals(decisions_.at(i),
                                                           command)) {
      logger_->debug("already executed command for slot {}, performing no-op",
                     i);
      ++slot_out_;
      return;
    }
  }
  logger_->info("executing command for slot {}", slot_out_);

  std::string value;
  if (command.operation() == Command_Operation_GET) {
    if (store_.find(command.key()) != store_.end()) {
      value = store_.at(command.key());
    }
  } else if (command.operation() == Command_Operation_PUT) {
    auto result = store_.insert({command.key(), command.value()});
    if (std::get<1>(result)) {
      value = store_.at(command.key());
    }
  }

  Response r;
  r.set_value(value);

  Message m;
  m.set_type(Message_MessageType_RESPONSE);
  m.mutable_message()->PackFrom(r);

  logger_->info("sending response to {}", command.client());
  dispatch_queue_.push(std::make_pair(command.client(), m));

  ++slot_out_;
}

}  // namespace paxos
}  // namespace process
