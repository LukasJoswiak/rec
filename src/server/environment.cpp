// Copyright 2019 Lukas Joswiak

#include "server/environment.hpp"

#include <iostream>

#include "paxos/replica.hpp"
#include "server/connection_manager.hpp"

Environment::Environment(ConnectionManager& manager, std::string& server_name)
    : manager_(manager),
      server_name_(server_name),
      replica_(queue_),
      acceptor_(queue_) {
  // Start message hander.
  std::thread(&Environment::Dispatcher, this).detach();

  // Spawn Paxos handlers.
  std::thread(&paxos::Replica::Run, &replica_).detach();
  std::thread(&paxos::Acceptor::Run, &acceptor_).detach();
}

void Environment::Deliver(const google::protobuf::Any& message,
                          const std::string& endpoint) {
  manager_.Deliver(message, endpoint);
}

void Environment::HandleRequest(Request& r, const std::string& from) {
  std::cout << "Received Request from client " << from << std::endl;
  std::cout << "  key: " << r.key() << ", value: " << r.value() << std::endl;
}

void Environment::Dispatcher() {
  while (1) {
    int front = queue_.front();
    std::cout << "Front of queue: " << front << std::endl;
    queue_.pop();
  }
}
