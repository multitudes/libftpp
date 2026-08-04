#pragma once

#include <deque>
#include <mutex>

template <typename TType> class ThreadSafeQueue {
private:
  std::deque<TType> _q;
  std::mutex _mutex;

public:
  ThreadSafeQueue() = default;
  ~ThreadSafeQueue() = default;

  // Best Practice: Delete copy constructors because std::mutex cannot be
  // copied!
  ThreadSafeQueue(const ThreadSafeQueue &) = delete;
  ThreadSafeQueue &operator=(const ThreadSafeQueue &) = delete;

  void push_back(const TType &newElement) {
    std::lock_guard<std::mutex> lock(_mutex);
    _q.push_back(newElement);
  }

  void push_front(const TType &newElement) {
    std::lock_guard<std::mutex> lock(_mutex);
    _q.push_front(newElement);
  }

  TType pop_back() {
    std::lock_guard<std::mutex> lock(_mutex);
    // Subject Hint: Must throw an exception if empty!
    if (_q.empty()) {
      throw std::runtime_error(
          "Error: Attempted to pop_back from an empty queue!");
    }
    TType value = _q.back();
    _q.pop_back();
    return value;
  }

  TType pop_front() {
    std::lock_guard<std::mutex> lock(_mutex);
    // Subject Hint: Must throw an exception if empty!
    if (_q.empty()) {
      throw std::runtime_error(
          "Error: Attempted to pop_front     from an empty queue!");
    }
    TType value = _q.front();
    _q.pop_front();
    return value;
  }
};
