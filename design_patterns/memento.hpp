#pragma once

#ifndef MEMENTO_HPP
#define MEMENTO_HPP

#include "../libftpp.hpp"

class Memento {
public:
  using Snapshot = DataBuffer;

  // Virtual destructor is mandatory for classes meant to be inherited
  virtual ~Memento() = default;

  Snapshot save();
  void load(const Snapshot &state);

private:
  // the child must provide
  virtual void _saveToSnapshot(Snapshot &snapshot) const = 0;
  virtual void _loadFromSnapshot(Snapshot &snapshot) = 0;
};

#endif