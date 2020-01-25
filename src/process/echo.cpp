// Copyright 2019 Lukas Joswiak

#include "process/echo.hpp"

#include <chrono>
#include <thread>

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
  if (pinged_.find(from) == pinged_.end()) {
    // TODO: send message to leader with all online servers.
    logger_->info("{} connected", from);
  }
  pinged_[from] = true;
}

void Echo::HeartbeatTimer() {
  Message message;
  message.set_type(Message_MessageType_HEARTBEAT);
  message.set_from(address_);

  dispatch_queue_.push(std::make_pair(std::nullopt, message));

  std::thread([this]() {
    std::this_thread::sleep_for(kHeartbeatInterval);
    HeartbeatTimer();
  }).detach();
}

void Echo::HeartbeatCheckTimer() {
  for (auto& [key, value] : pinged_) {
    if (!value) {
      // TODO: send message to leader with all online servers.
      logger_->info("{} dead!", key);
      pinged_.erase(key);
    } else {
      value = false;
    }
  }

  std::thread([this]() {
    std::this_thread::sleep_for(kHeartbeatCheckInterval);
    HeartbeatCheckTimer();
  }).detach();
}

}  // namespace process
