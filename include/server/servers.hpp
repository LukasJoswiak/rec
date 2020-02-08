#ifndef INCLUDE_SERVER_SERVERS_HPP_
#define INCLUDE_SERVER_SERVERS_HPP_

#include <array>
#include <utility>

#include "server/code.hpp"

// Hardcode the list of servers for now...
const std::array<std::pair<std::string, uint16_t>, 5> kServers = {
  std::make_pair("server1", 1111),
  std::make_pair("server2", 1112),
  std::make_pair("server3", 1113),
  std::make_pair("server4", 1114),
  std::make_pair("server5", 1115)
};

// With 5 nodes, data split into three original chunks and one tolerated
// failure, quorum size will be 4.
const int kToleratedFailures = 1;

const int kQuorum = Code::kOriginalBlocks + kToleratedFailures;

#endif  // INCLUDE_SERVER_SERVERS_HPP_
