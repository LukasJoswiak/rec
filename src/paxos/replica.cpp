// Copyright 2019 Lukas Joswiak

#include "paxos/replica.hpp"

namespace paxos {

Replica::Replica(common::SharedQueue<Message>& message_queue,
                 common::SharedQueue<Message>& dispatch_queue)
    : Process(message_queue, dispatch_queue),
      slot_in_(1),
      slot_out_(1) {}

void Replica::Handle(Message&& message) {
  if (message.type() == Message_MessageType_REQUEST) {
    Request r;
    message.message().UnpackTo(&r);
    HandleRequest(r, message.from());
  }
}

void Replica::HandleRequest(Request& r, const std::string& from) {
  std::cout << "Received Request from " << from << std::endl;
  std::cout << "  key: " << r.key() << ", value: " << r.value() << std::endl;
}

}  // namespace paxos
