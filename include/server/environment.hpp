// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_SERVER_ENVIRONMENT_HPP_
#define INCLUDE_SERVER_ENVIRONMENT_HPP_

#include <google/protobuf/any.pb.h>

#include "paxos/acceptor.hpp"
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

  // Sends the given message to the given endpoint.
  void Deliver(const google::protobuf::Any& message,
               const std::string& endpoint);

  void HandleRequest(Request& r, const std::string& from);

 private:
  // Attempts delivery of messages added to the shared queue. This function
  // blocks while the queue is empty, and so should be run on its own thread.
  void Dispatcher();

  ConnectionManager& manager_;
  std::string& server_name_;

  paxos::Replica replica_;
  paxos::Acceptor acceptor_;

  // Threads push messages onto the shared queue to be sent.
  common::SharedQueue<int> queue_;
};

#endif  // INCLUDE_SERVER_ENVIRONMENT_HPP_
