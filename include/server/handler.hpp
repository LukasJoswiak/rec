// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_SERVER_HANDLER_HPP_
#define INCLUDE_SERVER_HANDLER_HPP_

#include <google/protobuf/any.pb.h>

#include <memory>
#include <string>

#include <boost/asio.hpp>

#include "server/environment.hpp"

// Forward declare ConnectionManager to break circular dependency.
class ConnectionManager;

// Converts messages received on the network into concrete message types and
// forwards to the appropriate handler.
class Handler {
 public:
  Handler(ConnectionManager& manager, std::string& server_name);
  Handler(const Handler& other) = delete;
  Handler& operator=(const Handler& other) = delete;

  // Parses message into appropriate message type and calls the correct handler.
  void Handle(const std::string& message, const std::string& from);

  // Parses message and calls correct handler.
  void Handle(const google::protobuf::Any& message,
              const std::string& from);

 private:
  Environment environment_;
};

#endif  // INCLUDE_SERVER_HANDLER_HPP_
