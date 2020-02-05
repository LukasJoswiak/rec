#ifndef INCLUDE_SERVER_SERVERS_HPP_
#define INCLUDE_SERVER_SERVERS_HPP_

#include <array>
#include <utility>

// Hardcode the list of servers for now...
const std::array<std::pair<std::string, uint16_t>, 5> kServers = {
  std::make_pair("server1", 1111),
  std::make_pair("server2", 1112),
  std::make_pair("server3", 1113),
  std::make_pair("server4", 1114),
  std::make_pair("server5", 1115)
};

#endif  // INCLUDE_SERVER_SERVERS_HPP_
