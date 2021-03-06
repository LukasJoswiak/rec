#ifndef INCLUDE_PAXOS_COMMANDER_HPP_
#define INCLUDE_PAXOS_COMMANDER_HPP_

#include <string>
#include <unordered_set>
#include <utility>

#include "proto/messages.pb.h"
#include "process/paxos/paxos_process.hpp"

namespace process {
namespace paxos {

class Commander : public PaxosProcess {
 public:
  Commander(
      common::SharedQueue<Message>& message_queue,
      common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
          dispatch_queue,
      std::string& address, BallotNumber& ballot_number, int slot_number,
      Command& command);
  ~Commander() override;

  void Run() override;

  void Handle(Message&& message) override;

 private:
  void HandleP2B(P2B&& p, const std::string& from);

  std::unordered_set<std::string> received_from_;

  BallotNumber ballot_number_;
  int slot_number_;
  Command command_;
};

}  // namespace paxos
}  // namespace process

#endif  // INCLUDE_PAXOS_COMMANDER_HPP_
