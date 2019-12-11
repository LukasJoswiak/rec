// Copyright 2019 Lukas Joswiak

#ifndef INCLUDE_PAXOS_SHARED_QUEUE_HPP_
#define INCLUDE_PAXOS_SHARED_QUEUE_HPP_

#include <condition_variable>
#include <deque>
#include <mutex>

namespace common {

// Thread safe queue implementation implemented with mutexes.
template <typename T>
class SharedQueue {
 public:
  SharedQueue();
  ~SharedQueue();

  SharedQueue(const SharedQueue& other) = delete;
  SharedQueue(SharedQueue&& other) = delete;

  // Returns the first element in the queue.
  T& front();

  // Removes an element from the front of the queue.
  void pop();

  // Inserts the given element at the end of the queue.
  void push(const T& item);
  void push(T&& item);

  // Returns the number of elements in the queue.
  int size();

 private:
  std::deque<T> queue_;
  std::mutex mutex_;
  std::condition_variable cv_;
};

}  // namespace common

#endif  // INCLUDE_PAXOS_SHARED_QUEUE_HPP_
