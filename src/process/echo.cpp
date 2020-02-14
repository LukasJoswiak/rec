#include "process/echo.hpp"

#include <thread>

#include "proto/internal.pb.h"
#include "proto/messages.pb.h"

namespace {
const std::chrono::milliseconds kHeartbeatInterval(100);
const std::chrono::milliseconds kHeartbeatCheckInterval(
    kHeartbeatInterval * 4);
}

namespace process {

Echo::Echo(
    common::SharedQueue<Message>& message_queue,
    common::SharedQueue<std::pair<std::optional<std::string>, Message>>&
        dispatch_queue,
    std::string& address)
    : Process(message_queue, dispatch_queue),
      address_(address) {
  logger_ = spdlog::get("echo");

  // Run timers and handlers on their own thread.
  StartTimer(std::bind(&Echo::HeartbeatTimer, this), kHeartbeatInterval);
  StartTimer(std::bind(&Echo::HeartbeatCheckTimer, this),
      kHeartbeatCheckInterval);
}

void Echo::Handle(Message&& message) {
  if (message.type() == Message_MessageType_HEARTBEAT) {
    Heartbeat h;
    message.message().UnpackTo(&h);
    HandleHeartbeat(std::move(h), message.from());
  }
}

void Echo::HandleHeartbeat(Heartbeat&& h, const std::string& from) {
  bool new_server = false;
  if (pinged_.find(from) == pinged_.end()) {
    logger_->info("{} connected", from);
    new_server = true;
  }
  pinged_[from] = true;

  if (new_server) {
    SendUpdate();
  }
}

void Echo::HeartbeatTimer() {
  Message m;
  m.set_type(Message_MessageType_HEARTBEAT);
  m.set_from(address_);

  dispatch_queue_.push(std::make_pair(std::nullopt, m));
}

void Echo::HeartbeatCheckTimer() {
  for (auto it = pinged_.begin(); it != pinged_.end();) {
    if (!it->second) {
      logger_->info("{} disconnected", it->first);

      it = pinged_.erase(it);
      SendUpdate();
    } else {
      it->second = false;
      ++it;
    }
  }
}

void Echo::SendUpdate() {
  Status s;
  for (const auto& it : pinged_) {
    s.add_live(it.first);
  }

  Message m;
  m.set_type(Message_MessageType_STATUS);
  m.mutable_message()->PackFrom(s);

  dispatch_queue_.push(std::make_pair(address_, m));
}

void Echo::StartTimer(std::function<void(void)> function,
    const std::chrono::milliseconds interval) {
  std::thread([function, interval]() {
    while (true) {
      std::this_thread::sleep_for(interval);
      function();
    }
  }).detach();
}

}  // namespace process
