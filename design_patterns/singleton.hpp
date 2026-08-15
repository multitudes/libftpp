#pragma once

#include <stdexcept> // Replaced <exception> to use std::runtime_error
#include <utility>   // Required for std::forward

// when we pass a class as TType, this class needs to use the friend keyword
// and have a private constructor
template <typename TType> class Singleton {
private:
  static TType *_instance;

  // Optional but best practice
  Singleton() = default;
  ~Singleton() = default;

public:
  static TType *instance() { return _instance; }

  template <typename... TArgs> static void instantiate(TArgs &&...p_args) {
    if (_instance != nullptr) {
      throw std::runtime_error(
          "Error: Singleton instance already initialized!"); //?
    }
    _instance = new TType(std::forward<TArgs>(p_args)...);
  }
};

// A rule in C++ about static class variables: you have to actually allocate
// their memory outside of the class definition in your header file.
template <typename TType> TType *Singleton<TType>::_instance = nullptr;
