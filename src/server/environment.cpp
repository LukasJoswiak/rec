// Copyright 2019 Lukas Joswiak

#include "server/environment.hpp"

#include <iostream>

#include "server/connection_manager.hpp"

Environment::Environment(ConnectionManager& manager, std::string& server_name)
    : manager_(manager), server_name_(server_name) {}

void Environment::Deliver(const google::protobuf::Any& message,
                          const std::string& endpoint) {
  manager_.Deliver(message, endpoint);
}

void Environment::HandleRequest(Request& r, const std::string& from) {
  std::cout << "Received Request from client " << from << std::endl;
  std::cout << "  key: " << r.key() << ", value: " << r.value() << std::endl;
}
