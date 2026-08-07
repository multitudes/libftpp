#include "chronometer.hpp"
#include <chrono>

Chronometer::Chronometer() {};
void Chronometer::start() {
  _start = std::chrono::steady_clock::now();
  _is_running = true;
};
void Chronometer::stop() { _end = std::chrono::steady_clock::now(); };

double Chronometer::getElapsedSeconds() const {
  std::chrono::steady_clock::time_point current_end =
      _is_running ? std::chrono::steady_clock::now() : _end;
  std::chrono::duration<double> elapsed = current_end - _start;
  return elapsed.count();
};