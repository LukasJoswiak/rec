#ifndef INCLUDE_PROCESS_ECHO_HPP_
#define INCLUDE_PROCESS_ECHO_HPP_

#include <unordered_map>

#include "process/process.hpp"
#include "proto/messages.pb.h"

namespace process {

// Runs as a thread receiving and broadcasting heartbeat messages. Servers
// determine the state of the system through heartbeats (or the lack thereof)
// from other servers.
class Echo : public Process {
 public:
  Echo(common::SharedQueue<Message>& message_queue,
       common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
           dispatch_queue,
       std::string& address);

  void Handle(Message&& message) override;

 private:
  void HandleHeartbeat(Heartbeat&& h, const std::string& from);

  // Timer function used to rebroadcast heartbeat periodically.
  void HeartbeatTimer();
  // Timer function used to check server liveness periodically.
  void HeartbeatCheckTimer();

  // Sends an update message containing live servers. Delivered locally to
  // another thread, not sent over the network.
  void SendUpdate();

  std::string address_;

  // Map of server to whether it has pinged within the last interval. If not,
  // it should be considered dead.
  std::unordered_map<std::string, bool> pinged_;
};

}  // namespace process

#endif  // INCLUDE_PROCESS_ECHO_HPP_
