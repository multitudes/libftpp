#pragma once

#include "../thread/thread_safe_queue.hpp"
#include "thread.hpp"
#include <atomic>
#include <memory>
#include <vector>

class WorkerPool {
private:
  ThreadSafeQueue<std::function<void()>> _workersQueue;
  std::vector<std::unique_ptr<Thread>> _workers;

  // (Tells perpetual threads when to stop) We use std::atomic so multiple
  // threads can read it safely without a mutex.
  std::atomic<bool> _isRunning;

public:
  WorkerPool(size_t numWorkers);
  ~WorkerPool();
  void addJob(const std::function<void()> &jobToExecute);

  class IJobs {
  public:
    virtual ~IJobs() = default;
    virtual void execute() = 0;
  };
};
