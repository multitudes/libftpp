#pragma once

#include <functional>
#include <map>
#include <set>
#include <stdexcept>
#include <utility> // to get access to std::pair.

// if using a Class as TState needs to implement the `<` operator
template <typename TState> class StateMachine {

private:
  TState _currentState;
  std::set<TState> _allStates;
  std::map<TState, std::function<void()>> _actions;
  std::map<std::pair<TState, TState>, std::function<void()>> _transitions;
  bool _initialized = false;

public:
  StateMachine() = default;
  ~StateMachine() = default;

  void addState(const TState &state) {
    _allStates.insert(state);
    // If this is the very first state added, make it the default starting state
    if (!_initialized) {
      _currentState = state;
      _initialized = true;
    }
  }

  void addTransition(const TState &startState, const TState &finalState,
                     const std::function<void()> &lambda) {
    _transitions[{startState, finalState}] = lambda;
  }

  void addAction(const TState &state, const std::function<void()> &lambda) {
    _actions[state] = lambda;
  }

  void transitionTo(const TState &newState) {
    // throws if the action is not set up
    if (!_initialized) {
      throw std::invalid_argument("StateMachine has no states initialized!");
    }
    std::pair<TState, TState> transitionKey = {_currentState, newState};
    auto it = _transitions.find(transitionKey);
    if (it == _transitions.end()) {
      throw std::invalid_argument("Invalid transition attempted!");
    }
    it->second();
    _currentState = newState;
  }

  void update() {
    if (!_initialized) {
      throw std::runtime_error("StateMachine has no states initialized!");
    }
    auto it = _actions.find(_currentState);
    if (it == _actions.end()) {
      throw std::invalid_argument(
          "No action registered for the current state!");
    }

    // Execute the action lambda for the current state
    it->second();
  }
};
