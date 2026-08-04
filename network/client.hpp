#pragma once

#include "../thread/thread.hpp"
#include "message.hpp"
#include <atomic>
#include <cstdlib>
#include <functional>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <sys/socket.h>

class Client {
private:
  int _socketFd;
  std::atomic<bool> _isConnected;
  std::mutex _mutex;
  std::map<Message::Type, std::function<void(const Message &msg)>> _actions;
  std::queue<Message> _inbox;
  std::unique_ptr<Thread> _listenerThread;

  void _listenLoop();

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