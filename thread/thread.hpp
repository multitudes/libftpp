#pragma once

#ifndef THREAD_HPP
#define THREAD_HPP

#include "../iostream/thread_safe_iostream.hpp"
#include <functional>
#include <string>
#include <thread>

class Thread {
private:
  std::string _name;
  std::function<void()> _functToExecute;
  std::thread _thread;

public:
  Thread(const std::string &name, std::function<void()> functToExecute)
      : _name(name), _functToExecute(functToExecute) {}
  ~Thread() = default;

  void start() {
    _thread = std::thread([this]() {
      threadSafeCout.setPrefix(this->_name);
      this->_functToExecute();
    });
  }

  void stop() {}
};

#endif