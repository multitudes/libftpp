#pragma once

#include "thread.hpp"
#include <functional>
#include <map>
#include <mutex>
#include <string>

class PersistentWorker {
public:
  PersistentWorker();
  ~PersistentWorker();

  void addTask(const std::string &name,
               const std::function<void()> &jobToExecute);
  void removeTask(const std::string &name);

private:
  std::mutex _mutex;
  std::atomic<bool> _isRunning;
  std::map<std::string, std::function<void()>> _tasks;
  std::unique_ptr<Thread> _worker;
};
