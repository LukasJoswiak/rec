#ifndef INCLUDE_PAXOS_COMPARE_HPP_
#define INCLUDE_PAXOS_COMPARE_HPP_

#include <functional>

#include "google/protobuf/util/message_differencer.h"
#include "proto/messages.pb.h"

namespace process {
namespace paxos {

// Custom hash and equals function for BallotNumber and PValue messages. Used to
// enable inclusion in standard library unordered containers.

struct BallotHash {
  std::size_t operator()(const BallotNumber& ballot) const noexcept {
    return std::hash<int>()(ballot.number());
  }
};

struct BallotEqualTo {
  bool operator()(const BallotNumber& b1, const BallotNumber& b2) const {
    return google::protobuf::util::MessageDifferencer::Equals(b1, b2);
  }
};

struct PValueHash {
  std::size_t operator()(const PValue& pvalue) const noexcept {
    return std::hash<int>()(pvalue.slot_number());
  }
};

struct PValueEqualTo {
  bool operator()(const PValue& p1, const PValue& p2) const {
    return google::protobuf::util::MessageDifferencer::Equals(p1, p2);
  }
};

}  // namespace paxos
}  // namespace process

#endif  // INCLUDE_PAXOS_COMPARE_HPP_
