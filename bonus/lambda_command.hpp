#pragma once
#pragma once
#include "command.hpp"
#include <functional>

// same as before in the command.hpp
// class Command {
// public:
//   virtual ~Command() = default;
//   virtual void execute() = 0;
// };

class LambdaCommand : public Command {
private:
  std::function<void()> _action; // Can hold literally any block of code

public:
  LambdaCommand(std::function<void()> action) : _action(action) {}

  void execute() override {
    if (_action) {
      _action();
    }
  }
};