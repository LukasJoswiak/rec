#ifndef INCLUDE_PAXOS_PROCESS_HPP_
#define INCLUDE_PAXOS_PROCESS_HPP_

#include "process/process.hpp"

namespace process {
namespace paxos {

class PaxosProcess : public Process {
 public:
  PaxosProcess(
      common::SharedQueue<Message>& message_queue,
      common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
          dispatch_queue,
      std::string& address);
 protected:
  // Returns a negative number if b1 > b2, a positive number if b1 < b2, or
  // zero if b1 equals b2. Ballot numbers are totally ordered and are compared
  // first using their numbers, then using the server address for tie-breakers.
  int CompareBallotNumbers(const BallotNumber& b1, const BallotNumber& b2);

  // The address of this server.
  std::string address_;
};

}  // namespace paxos
}  // namespace process

#endif  // INCLUDE_PAXOS_PROCESS_HPP_
