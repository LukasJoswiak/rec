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

  // Attempts to pop an element off the front of the queue, returning it in the
  // output parameter `element`. Blocks until the queue has an element or is
  // stopped. Returns true if an element was popped successfully, or false if
  // the queue has been stopped.
  bool try_pop(T* element);

  // Inserts the given element at the end of the queue.
  void push(const T& item);
  void push(T&& item);

  // Returns the number of elements in the queue.
  int size();

  // Returns true if the queue is empty.
  bool empty();

  // Cleans up state and stops condition variables.
  void stop();

 private:
  std::deque<T> queue_;
  std::mutex mutex_;
  std::condition_variable cv_;

  // True if the queue can accept requests.
  bool accept_;
};

// Function definitions must be declared in same file so functions with
// appropriate types can be generated at compile time.

template <typename T>
SharedQueue<T>::SharedQueue() : accept_(true) {}

template <typename T>
SharedQueue<T>::~SharedQueue() {}

template <typename T>
bool SharedQueue<T>::try_pop(T* element) {
  std::unique_lock<std::mutex> lock(mutex_);

  // Wait until the queue has an element or should be shut down.
  cv_.wait(lock, [this]() {
    return !queue_.empty() || !accept_;
  });

  if (!queue_.empty()) {
    *element = queue_.front();
    queue_.pop_front();
  }

  return accept_;
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

template <typename T>
void SharedQueue<T>::stop() {
  accept_ = false;
  cv_.notify_all();
}

}  // namespace common
}  // namespace process

#endif  // INCLUDE_PAXOS_SHARED_QUEUE_HPP_
