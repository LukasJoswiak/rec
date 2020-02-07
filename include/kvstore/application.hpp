#ifndef INCLUDE_KVSTORE_APPLICATION_HPP_
#define INCLUDE_KVSTORE_APPLICATION_HPP_

#include <optional>

#include "kvstore/store.hpp"
#include "proto/messages.pb.h"

namespace kvstore {

// An at-most-once key-value store application. The simplifying assumption is
// made that each client will send at most one request at a time, waiting to
// send the next request until hearing the response. Responses are tracked
// per-client, and the application will not work if one client can have multiple
// outstanding requests at once.
class Application {
 public:
  Application();

  // Executes the command if the sequence number matches the expected sequence
  // number for the client. Returns the result in a Response, or std::nullopt if
  // the command was not executed. Guarantees at-most-once semantics.
  std::optional<Response> Execute(const Command& command);

  // Returns true if the command has already been executed.
  bool Executed(const Command& command);

 private:

  Store<std::string, std::string> store_;

  // Map of client address to the highest sequence number seen from the client.
  std::unordered_map<std::string, int> sequence_numbers_;
  // Map of client address to the most recent result for the client.
  std::unordered_map<std::string, Response> results_;

  std::shared_ptr<spdlog::logger> logger_;
};

}  // namespace kvstore

#endif  // INCLUDE_KVSTORE_APPLICATION_HPP_
