// Copyright 2019 Lukas Joswiak

#include "paxos/shared_queue.hpp"

namespace common {

template <typename T>
SharedQueue<T>::SharedQueue() {}

template <typename T>
SharedQueue<T>::~SharedQueue() {}

template <typename T>
T& SharedQueue<T>::front() {
  std::unique_lock<std::mutex> lock(mutex_);
  while (queue_.empty()) {
    cv_.wait();
  }
  return queue_.front();
}

template <typename T>
void SharedQueue<T>::pop() {
  std::unique_lock<std::mutex> lock(mutex_);
  while (queue_.empty()) {
    cv_.wait();
  }
  queue_.pop_front();
}

template <typename T>
void SharedQueue<T>::push(const T& item) {
  std::unique_lock<std::mutex> lock(mutex_);
  queue_.push_back(item);
  lock.unlock();
  cv_.notify_one();
}

template <typename T>
void SharedQueue<T>::push(T&& item) {
  std::unique_lock<std::mutex> lock(mutex_);
  // Transfer ownership of the object to the queue.
  queue_.push_back(std::move(item));
  lock.unlock();
  cv_.notify_one();
}

template <typename T>
int SharedQueue<T>::size() {
  const std::lock_guard<std::mutex> lock(mutex_);
  return queue_.size();
}

}  // namespace common
