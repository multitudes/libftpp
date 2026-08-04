#pragma once

#include "../thread/thread.hpp" // Needed for std::unique_ptr<Thread>
#include "message.hpp"
#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <poll.h> // Needed for struct pollfd
#include <queue>
#include <vector>

class Server {
private:
  std::vector<struct pollfd> _pollfds;
  std::atomic<bool> _isRunning;
  std::map<Message::Type,
           std::function<void(long long &clientID, const Message &msg)>>
      _actions;
  std::unique_ptr<Thread> _listenerThread;
  std::mutex _mutex;
  int _serverSocketFd;
  // keep track of clientID to fds
  std::map<long long, int> clientList;

  // increments every time a new user connects, guaranteeing everyone gets a
  // unique ID.
  long long _nextClientID = 1;
  //   Stores both the sender's ID and the message
  std::queue<std::pair<long long, Message>> _inbox;
  // the poll array
  std::vector<struct pollfd> _pollFds;

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