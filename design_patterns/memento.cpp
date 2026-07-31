#include "memento.hpp"

Memento::Snapshot Memento::save() {
  Snapshot snapshot;
  _saveToSnapshot(snapshot);
  return snapshot;
}

void Memento::load(const Snapshot &state) {
  Snapshot stateCopy = state;
  _loadFromSnapshot(stateCopy);
}