#include "process/echo.hpp"

#include <chrono>
#include <thread>

#include "proto/internal.pb.h"
#include "proto/messages.pb.h"

namespace {
  const std::chrono::milliseconds kHeartbeatInterval(100);
  const std::chrono::milliseconds kHeartbeatCheckInterval(400);
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

  // Use a spawned thread to simulate an asynchronous timer. Would use a
  // boost::asio timer, but don't have acesss to the io_context in this class.
  // This timer will not run exactly on each interval due to instructions
  // taking time to run, but it should be close enough.
  std::thread([this]() {
    std::this_thread::sleep_for(kHeartbeatInterval);
    HeartbeatTimer();
  }).detach();

  std::thread([this]() {
    std::this_thread::sleep_for(kHeartbeatCheckInterval);
    HeartbeatCheckTimer();
  }).detach();
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

  std::thread([this]() {
    std::this_thread::sleep_for(kHeartbeatInterval);
    HeartbeatTimer();
  }).detach();
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

  std::thread([this]() {
    std::this_thread::sleep_for(kHeartbeatCheckInterval);
    HeartbeatCheckTimer();
  }).detach();
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

}  // namespace process
