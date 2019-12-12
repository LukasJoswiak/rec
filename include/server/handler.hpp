// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_SERVER_HANDLER_HPP_
#define INCLUDE_SERVER_HANDLER_HPP_

#include <google/protobuf/any.pb.h>

#include <memory>
#include <string>

#include <boost/asio.hpp>

#include "proto/heartbeat.pb.h"
#include "proto/messages.pb.h"

// Message handler for messages received on the replica.
class Handler {
 public:
  Handler();
  Handler(const Handler& other) = delete;
  Handler& operator=(const Handler& other) = delete;

  // Parses message into appropriate message type and calls the correct handler.
  void Handle(const std::string& message, const std::string& from) const;

  // Parses message and calls correct handler.
  void Handle(const google::protobuf::Any& message,
              const std::string& from) const;

 private:
  void HandleRequest(Request& r, const std::string& from) const;
};

#endif  // INCLUDE_SERVER_HANDLER_HPP_
