#pragma once

// can hold any command - interface
class Command {
public:
  virtual ~Command() = default;
  virtual void execute() = 0;
};

template <typename Receiver> class SimpleCommand : public Command {
private:
  Receiver *_receiver; // pointer to the object
  using Action = void (
      Receiver::*)(); // pointer to member function of Receiver - this is type
  Action _action;     // this is the pointer to the actual method
public:
  SimpleCommand(Receiver *receiver, Action action)
      : _receiver(receiver), _action(action) {};
  void execute() override {
    if (_receiver && _action) { // Safely check both pointers
      (_receiver->*_action)();
    }
  }
};