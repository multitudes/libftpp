#pragma once

#ifndef MEMENTO_HPP
#define MEMENTO_HPP

template <typename TEvent> class Observer {
public:
  void subscribe(const TEvent &event, const std::function<void()> &lambda);

  void notify(const TEvent &event);
};

#endif