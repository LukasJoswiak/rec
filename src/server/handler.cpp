// Copyright 2019 Lukas Joswiak

#include "server/handler.hpp"

#include <memory>

#include "paxos/replica.hpp"

Handler::Handler() {}

void Handler::Handle(const std::string& message,
                     const std::string& from) const {
  google::protobuf::Any any;
  any.ParseFromString(message);

  Handle(any, from);
}

void Handler::Handle(const google::protobuf::Any& message,
                     const std::string& from) const {
  Request r;
  if (message.UnpackTo(&r)) {
    HandleRequest(r, from);
  }
}

void Handler::HandleRequest(Request& r, const std::string& from) const {
  std::cout << "Received Request from client " << from << std::endl;
  std::cout << "  key: " << r.key() << ", value: " << r.value() << std::endl;
}
