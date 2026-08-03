#pragma once

#ifndef THREAD_HPP
#define THREAD_HPP

#include <functional>
#include <string>
#include <thread>

class Thread {
private:
  std::string _name;
  std::function<void()> _functToExecute;
  std::thread _thread;

public:
  Thread(const std::string &name, std::function<void()> functToExecute);
  ~Thread();

  void start();
  void stop();
};

#endif