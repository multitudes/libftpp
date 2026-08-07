#pragma once
#include <chrono>

class Timer {
private:
  std::chrono::steady_clock::time_point _start_time;
  std::chrono::milliseconds _duration;

public:
  Timer(long long duration_ms);

  void reset();
  bool hasTimedOut() const;
};