// Copyright 2019 Lukas Joswiak

#include "server/environment.hpp"

#include "server/connection_manager.hpp"

Environment::Environment(ConnectionManager& manager, std::string& server_name)
    : manager_(manager),
      server_name_(server_name),
      echo_(echo_queue_, dispatch_queue_, server_name_),
      replica_(replica_queue_, dispatch_queue_),
      acceptor_(acceptor_queue_, dispatch_queue_),
      leader_(leader_queue_, dispatch_queue_, server_name) {}

void Environment::Start() {
  // Start message handler.
  std::thread(&Environment::Dispatcher, this).detach();

  // Spawn echo handler.
  std::thread(&process::Echo::Run, &echo_).detach();

  // Spawn Paxos handlers.
  std::thread(&process::paxos::Replica::Run, &replica_).detach();
  std::thread(&process::paxos::Acceptor::Run, &acceptor_).detach();
  std::thread(&process::paxos::Leader::Run, &leader_).detach();
}

void Environment::Handle(const std::string& raw_message) {
  Message message;
  message.ParseFromString(raw_message);
  Handle(message);
}

void Environment::Handle(const Message& message) {
  switch (message.type()) {
    case Message_MessageType_HEARTBEAT:
      HandleEchoMessage(message);
    case Message_MessageType_REQUEST:
    case Message_MessageType_DECISION:
      HandleReplicaMessage(message);
      break;
    case Message_MessageType_P1A:
    case Message_MessageType_P2A:
      HandleAcceptorMessage(message);
      break;
    case Message_MessageType_PROPOSAL:
    case Message_MessageType_ADOPTED:
    case Message_MessageType_PREEMPTED:
    case Message_MessageType_P1B:
    case Message_MessageType_P2B:
    case Message_MessageType_STATUS:
      HandleLeaderMessage(message);
      break;
    default:
      break;
  }
}

void Environment::HandleEchoMessage(const Message& m) {
  echo_queue_.push(m);
}

void Environment::HandleReplicaMessage(const Message& m) {
  replica_queue_.push(m);
}

void Environment::HandleAcceptorMessage(const Message& m) {
  acceptor_queue_.push(m);
}

void Environment::HandleLeaderMessage(const Message& m) {
  leader_queue_.push(m);
}

void Environment::Dispatcher() {
  while (1) {
    auto pair = dispatch_queue_.front();
    dispatch_queue_.pop();

    Message message = std::get<1>(pair);
    message.set_from(server_name_);
    if (auto destination = std::get<0>(pair)) {
      manager_.Deliver(message, destination.value());
    } else {
      manager_.Broadcast(message);
    }
  }
}
