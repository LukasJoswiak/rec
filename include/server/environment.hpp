// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_SERVER_ENVIRONMENT_HPP_
#define INCLUDE_SERVER_ENVIRONMENT_HPP_

#include <string>

#include "paxos/acceptor.hpp"
#include "paxos/leader.hpp"
#include "paxos/replica.hpp"
#include "paxos/shared_queue.hpp"
#include "proto/messages.pb.h"

// Forward declare ConnectionManager to break circular dependency.
class ConnectionManager;

// State manager for the server. Handles messages parsed by the Handler and
// holds state for the server.
class Environment {
 public:
  Environment(ConnectionManager& manager, std::string& server_name);

  void HandleReplicaMessage(const Message& m);
  void HandleAcceptorMessage(const Message& m);
  void HandleLeaderMessage(const Message& m);

 private:
  // Attempts delivery of messages added to the shared queue. This function
  // blocks while the queue is empty, and so should be run on its own thread.
  void Dispatcher();

  ConnectionManager& manager_;
  std::string& server_name_;

  paxos::Replica replica_;
  paxos::Acceptor acceptor_;
  paxos::Leader leader_;

  // Queues used to pass messages to threads.
  common::SharedQueue<Message> replica_queue_;
  common::SharedQueue<Message> acceptor_queue_;
  common::SharedQueue<Message> leader_queue_;

  // Threads push messages onto a shared queue to enqueue them for delivery.
  // Pair consists of <optional(destination name), message>. If the destination
  // does not contain a value, the message will be broadcast to all servers.
  common::SharedQueue<std::pair<std::optional<std::string>, Message>>
      dispatch_queue_;
};

#endif  // INCLUDE_SERVER_ENVIRONMENT_HPP_
