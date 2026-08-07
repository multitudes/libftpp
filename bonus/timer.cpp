#include "timer.hpp"
#include <chrono>

Timer::Timer(long long duration_ms)
    : _duration{std::chrono::milliseconds{duration_ms}} {
  reset();
}
void Timer::reset() { _start_time = std::chrono::steady_clock::now(); }
bool Timer::hasTimedOut() const {
  auto now = std::chrono::steady_clock::now();
  return (now - _start_time >= _duration);
}