#include "thread.hpp"

Thread::Thread(const std::string &name, std::function<void()> functToExecute)
    : _name(name), _functToExecute(functToExecute) {}

Thread::~Thread() { stop(); };

void Thread::start() {
  _thread = std::thread([this]() {
    threadSafeCout.setPrefix("[" + this->_name + "] ");
    this->_functToExecute();
  });
}

void Thread::stop() {
  if (_thread.joinable()) {
    _thread.join();
    threadSafeCout << "stopping thread: " << std::this_thread::get_id()
                   << std::endl;
  }
}