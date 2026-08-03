#pragma once

#include "network.hpp"
#include <cstdlib>
#include <functional>
#include <string>

class Client {
private:
public:
  Client();
  ~Client();
  void connect(const std::string &address, const size_t &port);
  void disconnect();
  void defineAction(const Message::Type &messageType,
                    const std::function<void(const Message &msg)> &action);
  void send(const Message &message);
  void update();
};