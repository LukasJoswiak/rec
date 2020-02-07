#ifndef INCLUDE_PAXOS_SCOUT_HPP_
#define INCLUDE_PAXOS_SCOUT_HPP_

#include <string>
#include <unordered_set>
#include <utility>

#include "process/paxos/compare.hpp"
#include "process/paxos/paxos_process.hpp"
#include "proto/messages.pb.h"

namespace process {
namespace paxos {

class Scout : public PaxosProcess {
 public:
  Scout(
      common::SharedQueue<Message>& message_queue,
      common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
          dispatch_queue,
      std::string& address, BallotNumber& ballot_number);
  ~Scout() override;

  // Make the base class Run function available.
  using Process::Run;
  // Overload Process::Run and accept a scout ID used to identify which
  // scout instance the messages originated from.
  virtual void Run(int scout_id);

  void Handle(Message&& message) override;

  BallotNumber& ballot_number() {
    return ballot_number_;
  }

 private:
  void HandleP1B(P1B&& p, const std::string& from);

  BallotNumber ballot_number_;
  std::unordered_set<PValue, PValueHash, PValueEqualTo> pvalues_;
  std::unordered_set<std::string> received_from_;
};

}  // namespace paxos
}  // namespace process

#endif  // INCLUDE_PAXOS_SCOUT_HPP_
