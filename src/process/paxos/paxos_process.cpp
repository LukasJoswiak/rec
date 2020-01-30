// Copyright 2020 Lukas Joswiak

#include "process/paxos/paxos_process.hpp"

namespace process {
namespace paxos {

PaxosProcess::PaxosProcess(
    common::SharedQueue<Message>& message_queue,
    common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
        dispatch_queue)
    : Process(message_queue, dispatch_queue) {}

int PaxosProcess::CompareBallotNumbers(const BallotNumber& b1,
                                  const BallotNumber& b2) {
  if (b1.number() != b2.number()) {
    return b2.number() - b1.number();
  } else {
    if (b1.address().size() == 0 && b2.address().size() > 0) {
      return 1;
    }
    return b2.address().compare(b1.address());
  }
}

}  // namespace paxos
}  // namespace process
