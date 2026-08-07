#pragma once

#include <chrono>
class Chronometer {
private:
  std::chrono::steady_clock::time_point _start;
  std::chrono::steady_clock::time_point _end;
  bool _is_running = false;

public:
  Chronometer();
  void start();
  void stop();
  double getElapsedSeconds() const;
};
