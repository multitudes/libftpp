#include "worker_pool.hpp"
#include <chrono>
#include <thread>

WorkerPool::WorkerPool(size_t numWorkers) {
  _isRunning = true;
  // this is what the workers need to do
  auto workersInstructions = [this]() {
    while (this->_isRunning) {
      try {
        auto job = this->_workersQueue.pop_front();
        // if we did not get an exeption then we are still here to execute
        job();
      } catch (const std::exception &e) {
        // the queue was empty - sleep a moment and try again
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
      }
    }
    threadSafeCout << "Worker thread " << std::this_thread::get_id()
                   << " has cleanly exited its loop." << std::endl;
  };
  // put the workers to work on different threads
  for (size_t i = 0; i < numWorkers; ++i) {
    std::string workerName = "Worker " + std::to_string(i + 1) + " ";
    _workers.push_back(
        std::unique_ptr<Thread>(new Thread(workerName, workersInstructions)));
    _workers.back()->start();
  }
}

WorkerPool::~WorkerPool() {
  _isRunning = false;
  for (auto &worker : _workers) {
    worker->stop();
  }
}

void WorkerPool::addJob(const std::function<void()> &jobToExecute) {
  _workersQueue.push_back(jobToExecute);
}
