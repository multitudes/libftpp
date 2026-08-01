#pragma once

#ifndef SINGLETON_HPP
#define SINGLETON_HPP

template <typename TType> class Singleton {
private:
  static TType _instance;

public:
  TType *instance() {}

  template <typename... TArgs> void instantiate(TArgs &&...p_args) {}
};

#endif