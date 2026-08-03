#include "persistent_worker.hpp"
#include "../iostream/thread_safe_iostream.hpp"
#include <chrono>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <thread>

PersistentWorker::PersistentWorker() {
  _isRunning = true;
  // this is what the workers need to do
  auto workerTasks = [this]() {
    while (this->_isRunning) {
      std::unique_lock<std::mutex> lock(this->_mutex);
      if (this->_tasks.empty()) {
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(2));
        continue;
      }
      // execute all tasks - this is safer than one by one and then sleeping
      // because the iterator might invalidate in between
      for (auto const &pair : this->_tasks) {
        pair.second();
      }
      lock.unlock();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    threadSafeCout << "Worker thread " << std::this_thread::get_id()
                   << " has cleanly exited its loop." << std::endl;
  };

  _worker =
      std::unique_ptr<Thread>(new Thread("persistentWorker", workerTasks));

  _worker->start();
}

PersistentWorker::~PersistentWorker() {
  _isRunning = false;
  _worker->stop();
}

void PersistentWorker::addTask(const std::string &name,
                               const std::function<void()> &jobToExecute) {
  std::lock_guard<std::mutex> lock(this->_mutex);
  _tasks[name] = jobToExecute;
}
void PersistentWorker::removeTask(const std::string &name) {
  std::lock_guard<std::mutex> lock(this->_mutex);
  size_t removedCount = this->_tasks.erase(name);
  if (removedCount > 0) {
    // The task was successfully removed
    threadSafeCout << "Main thread " << std::this_thread::get_id() << name
                   << " removed." << std::endl;
  } else {
    // The task wasn't in the map to begin with
    threadSafeCout << "Main thread " << std::this_thread::get_id() << name
                   << " was not in the task list." << std::endl;
  }
}