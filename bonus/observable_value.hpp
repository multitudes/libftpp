#pragma once

#include <functional>
#include <vector>

template <typename T> class ObservableValue {
private:
  T _value;
  std::vector<std::function<void(const T &)>> _subscribers;

public:
  // constr
  ObservableValue(T initial_value = T()) : _value(initial_value) {}

  void subscribe(std::function<void(const T &)> callback) {
    _subscribers.push_back(callback);
  }

  const T &get() const { return _value; }
  void set(const T &newValue) {
    _value = newValue;
    for (const auto &callback : _subscribers) {
      callback(_value);
    }
  }
  // cosmetic - overload the = for set
  ObservableValue<T> &operator=(const T &newValue) {
    set(newValue);
    return *this;
  }
};