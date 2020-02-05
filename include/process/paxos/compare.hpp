#ifndef INCLUDE_PAXOS_COMPARE_HPP_
#define INCLUDE_PAXOS_COMPARE_HPP_

#include <functional>

#include "google/protobuf/util/message_differencer.h"
#include "proto/messages.pb.h"

namespace process {
namespace paxos {

// Custom hash and equals function for the PValue message. Used to enable PValue
// inclusion in standard library unordered containers.

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
