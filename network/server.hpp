#pragma once

#include "network.hpp"
#include <cstdlib>

class Server {
private:
  std::map<Message::Type, std::function<void(const Message &msg)>> _actions;

public:
  Server();
  ~Server();
  void start(const size_t &p_port);
  void defineAction(const Message::Type &messageType,
                    const std::function<void(long long &clientID,
                                             const Message &msg)> &action);
  void sendTo(const Message &message, long long clientID);
  void sendToArray(const Message &message, std::vector<long long> clientIDs);
  void sendToAll(const Message &message);
  void update();
};