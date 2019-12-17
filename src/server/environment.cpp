// Copyright 2019 Lukas Joswiak

#include "server/environment.hpp"

#include <iostream>

#include "paxos/replica.hpp"
#include "server/connection_manager.hpp"

Environment::Environment(ConnectionManager& manager, std::string& server_name)
    : manager_(manager),
      server_name_(server_name),
      replica_(replica_queue_, dispatch_queue_),
      acceptor_(acceptor_queue_, dispatch_queue_) {
  // Start message handler.
  std::thread(&Environment::Dispatcher, this).detach();

  // Spawn Paxos handlers.
  std::thread(&paxos::Replica::Run, &replica_).detach();
  std::thread(&paxos::Acceptor::Run, &acceptor_).detach();
}

void Environment::Deliver(const Message& message, const std::string& endpoint) {
  manager_.Deliver(message, endpoint);
}

void Environment::HandleReplicaMessage(const Message& m) {
  replica_queue_.push(m);
}

void Environment::Dispatcher() {
  while (1) {
    int front = dispatch_queue_.front();
    std::cout << "Front of queue: " << front << std::endl;
    dispatch_queue_.pop();
  }
}
