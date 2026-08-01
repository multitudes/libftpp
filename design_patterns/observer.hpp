#pragma once

#ifndef OBSERVER_HPP
#define OBSERVER_HPP

#include <functional>
#include <map>
#include <vector>

template <typename TEvent> class Observer {
private:
  // Key: TEvent (e.g., LEVEL_UP)
  // Value: A list of lambdas to execute
  std::map<TEvent, std::vector<std::function<void()>>> _subscribers;

public:
  Observer() = default;
  ~Observer() = default;

  // Subscribe a lambda to a specific event
  void subscribe(const TEvent &event, const std::function<void()> &lambda) {
    _subscribers[event].push_back(lambda);
  }

  // Notify all subscribers of an event
  // The 'const' here promises we won't change the dictionary
  void notify(const TEvent &event) const {
    auto it = _subscribers.find(event);

    if (it != _subscribers.end()) {
      for (const auto &lambda : it->second) {
        lambda();
      }
    }
  }
};

#endif