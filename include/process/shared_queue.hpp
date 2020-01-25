// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_PAXOS_SHARED_QUEUE_HPP_
#define INCLUDE_PAXOS_SHARED_QUEUE_HPP_

#include <condition_variable>
#include <deque>
#include <mutex>
#include <utility>

namespace process {
namespace common {

// Thread safe queue implementation implemented with mutexes.
template <typename T>
class SharedQueue {
 public:
  SharedQueue();
  ~SharedQueue();

  SharedQueue(const SharedQueue<T>& other) = delete;
  SharedQueue(SharedQueue<T>&& other) = delete;

  // Returns the first element in the queue. Blocks until an element is
  // available.
  T& front();

  // Removes an element from the front of the queue. Blocks if the queue is
  // empty, until an element can be removed.
  void pop();

  // Inserts the given element at the end of the queue.
  void push(const T& item);
  void push(T&& item);

  // Returns the number of elements in the queue.
  int size();

  // Returns true if the queue is empty.
  bool empty();

 private:
  std::deque<T> queue_;
  std::mutex mutex_;
  std::condition_variable cv_;
};

// Function definitions must be declared in same file so functions with
// appropriate types can be generated at compile time.

template <typename T>
SharedQueue<T>::SharedQueue() {}

template <typename T>
SharedQueue<T>::~SharedQueue() {}

template <typename T>
T& SharedQueue<T>::front() {
  std::unique_lock<std::mutex> lock(mutex_);
  while (queue_.empty()) {
    cv_.wait(lock);
  }
  return queue_.front();
}

template <typename T>
void SharedQueue<T>::pop() {
  std::unique_lock<std::mutex> lock(mutex_);
  while (queue_.empty()) {
    cv_.wait(lock);
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
  std::unique_lock<std::mutex> lock(mutex_);
  return queue_.size();
}

template <typename T>
bool SharedQueue<T>::empty() {
  std::unique_lock<std::mutex> lock(mutex_);
  return queue_.empty();
}

}  // namespace common
}  // namespace process

#endif  // INCLUDE_PAXOS_SHARED_QUEUE_HPP_
