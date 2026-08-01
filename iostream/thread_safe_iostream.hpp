#pragma once

#include <ostream>
#ifndef THREAD_SAFE_IOSTREAM_HPP
#define THREAD_SAFE_IOSTREAM_HPP

#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

class ThreadSafeIOStream {
private:
  std::string _prefix;
  std::ostringstream _buffer;

  static std::mutex _io_mutex;

public:
  ThreadSafeIOStream() = default;
  ~ThreadSafeIOStream() = default;

  void setPrefix(const std::string &prefix) { _prefix = prefix; }

  // Overload for << (Values)
  // Buffers the data locally. No locking needed yet!
  template <typename T> ThreadSafeIOStream &operator<<(const T &value) {
    _buffer << value;
    return *this;
  }
  // Overload for << (Manipulators like std::endl)
  // This is our trigger to lock the console and flush the buffer!
  ThreadSafeIOStream &operator<<(std::ostream &(*manip)(std::ostream &)) {
    std::lock_guard<std::mutex> lock(_io_mutex);

    // Print prefix, then the buffered text, then apply the manipulator
    std::cout << _prefix << _buffer.str();
    manip(std::cout);
    // Clear our personal buffer for the next message
    _buffer.str("");
    _buffer.clear();

    return *this;
  }

  // Overload for >> (Input)
  template <typename T> ThreadSafeIOStream &operator>>(T &value) {
    std::lock_guard<std::mutex> lock(_io_mutex);
    std::cin >> value;
    return *this;
  }
  // Prompt Method
  template <typename T> void prompt(const std::string &question, T &dest) {
    std::lock_guard<std::mutex> lock(_io_mutex);
    std::cout << _prefix << question;
    std::cin >> dest;
  }
};

// Subject Hint: Provide an equivalent to std::cout so there's no need to
// create a custom iostream.
// We declare it 'extern' here so all files can see it, but we build it in the
// .cpp file.
extern thread_local ThreadSafeIOStream threadSafeCout;

#endif