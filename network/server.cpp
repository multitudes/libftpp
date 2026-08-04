#include "server.hpp"
#include "message.hpp"
#include <cstddef>
#include <iostream>
#include <memory>
#include <mutex>
#include <netinet/in.h>
#include <queue>
#include <sys/_endian.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

Server::Server() : _serverSocketFd(-1), _isRunning(false) {}

Server::~Server() {
  // 1. Tell the background thread to break its while loop
  std::cout << "[Server] Stopping the poll loop\n" << std::endl;
  _isRunning = false;

  // 2. Safely stop and destroy the thread
  if (_listenerThread) {
    std::cout << "[Server] Stopping the thread " << _listenerThread << "\n "
              << std::endl;
    _listenerThread->stop();
    _listenerThread.reset();
  }

  // 3. Close the master server socket
  if (_serverSocketFd != -1) {
    std::cout << "[Server] Close the server socket\n" << std::endl;
    close(_serverSocketFd);
    _serverSocketFd = -1;
  }

  // 4. Close any remaining client sockets so we don't leak file descriptors!
  std::lock_guard<std::mutex> lock(_mutex);
  for (auto const &pair : clientList) {
    std::cout << "[Server] Close the client sockets" << pair.second << "\n "
              << std::endl;
    close(pair.second);
  }
  clientList.clear();
  std::cout << "[Server] Clearing client list\n " << std::endl;
  std::cout << "[Server] Clearing poll fd list\n " << std::endl;
  _pollFds.clear();
}

void Server::start(const size_t &p_port) {
  _serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
  if (_serverSocketFd < 0) {
    std::cerr << "[Server] Socket creation failed\n";
    return;
  }
  // allow the port to be reused immediately after a restart
  int opt = 1;
  if (setsockopt(_serverSocketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) <
      0) {
    std::cerr << "[Server] setsockopt failed\n";
    return;
  }
  // bind to the port
  struct sockaddr_in serv_addr;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY; // listen to any IP addr
  serv_addr.sin_port = htons(p_port);

  if (bind(_serverSocketFd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <
      0) {
    std::cerr << "[Server] Bind failed on port " << p_port << "\n";
    return;
  }
  // start listening
  if (listen(_serverSocketFd, SOMAXCONN) < 0) {
    std::cerr << "[Server] Listen failed\n";
    return;
  }
  _isRunning = true;
  std::cout << "[Server] Started listening on port " << p_port << "\n";

  // Setup Poll and start the background thread!
  _pollFds.push_back({_serverSocketFd, POLLIN, 0});
  _listenerThread =
      std::unique_ptr<Thread>(new Thread("Server listener", [this]() {
        while (_isRunning) {
          int poll_result = ::poll(_pollFds.data(), _pollFds.size(),
                                   100); // 100 is in ms timeout
          if (poll_result < 0) {
            // poll failed - could be an interrupt signal
            continue;
          }
          if (poll_result == 0) {
            // timeout
            continue;
          }
          // there is data!
          for (size_t i = 0; i < _pollFds.size(); ++i) {
            if (_pollFds[i].revents & POLLIN) {
              if (_pollFds[i].fd == _serverSocketFd) {
                // NEW client is connecting.
                int new_fd = ::accept(_serverSocketFd, nullptr, nullptr);
                if (new_fd > 0) {
                  std::lock_guard<std::mutex> lock(_mutex);
                  clientList[_nextClientID] = new_fd;
                  _pollFds.push_back({new_fd, POLLIN, 0});
                  std::cout << "[Server] Client " << _nextClientID
                            << " connected.\n";
                  _nextClientID++;
                }
              } else {
                // A regular client socket woke up. An
                // existing user sent a Message.
                int client_fd = _pollFds[i].fd;
                long long senderID = -1;
                {
                  std::lock_guard<std::mutex> lock(_mutex);
                  for (auto const pair : clientList) {
                    if (pair.second == client_fd) {
                      senderID = pair.first;
                      break;
                    }
                  }
                }
                int type;
                ssize_t bytes = ::recv(client_fd, &type, sizeof(int), 0);
                if (bytes < 0) {
                  std::cerr << "Failed to receive message type\n";
                  continue;
                }
                if (bytes == 0) {
                  std::cout << "[Server] Client " << senderID
                            << " disconnected.\n";
                  close(client_fd);
                  std::lock_guard<std::mutex> lock(_mutex);
                  clientList.erase(senderID);
                  _pollFds.erase((_pollFds.begin() + i));
                  i--;
                  continue;
                }
                uint32_t payloadSize;
                bytes = ::recv(client_fd, &payloadSize, sizeof(uint32_t), 0);
                if (bytes < 0) {
                  std::cerr << "Failed to receive message size\n";
                  continue;
                }
                if (bytes <= 0) { // disconnect
                  std::cout << "[Server] Client " << senderID
                            << " disconnected.\n";
                  close(client_fd);
                  std::lock_guard<std::mutex> lock(_mutex);
                  clientList.erase(senderID);
                  _pollFds.erase((_pollFds.begin() + i));
                  i--;
                  continue;
                }
                payloadSize = ntohl(payloadSize);
                type = ntohl(type);
                Message msg(type);

                std::vector<uint8_t> tempBuffer(payloadSize);
                size_t totalRead = 0;

                while (totalRead < payloadSize) {
                  ssize_t r = ::recv(client_fd, tempBuffer.data() + totalRead,
                                     payloadSize - totalRead, 0);

                  if (r > 0) {
                    totalRead += r;
                  } else if (r == 0) {
                    break;
                  }
                }
                msg.setBuffer(tempBuffer);
                std::lock_guard<std::mutex> lock(_mutex);
                _inbox.push(std::make_pair(senderID, msg));
              }
            }
          }
        }
      }));
  _listenerThread->start();
}

void Server::defineAction(
    const Message::Type &messageType,
    const std::function<void(long long &clientID, const Message &msg)>
        &action) {
  _actions[messageType] = action;
}

void Server::sendTo(const Message &message, long long clientID) {
  int fd = -1;
  std::unique_lock<std::mutex> lock(_mutex);
  auto it = clientList.find(clientID);
  if (it != clientList.end()) {
    fd = it->second;
  }
  // send
  lock.unlock();
  if (!_isRunning || fd == -1)
    return;
  //   Send the Message Type
  int networkType = htonl(message.type());
  ssize_t s1 = ::send(fd, &networkType, sizeof(int), 0);
  if (s1 < 0) {
    std::cerr << "Failed to send message type\n";
    return;
  }

  // Send the Payload Size
  // size_t is 64-bit on many systems, but network protocols usually use 32-bit
  // for sizes. We cast it to uint32_t before calling htonl to ensure both sides
  // agree on 4 bytes.
  uint32_t payloadSize = static_cast<uint32_t>(message.size());
  uint32_t networkSize = htonl(payloadSize);
  ssize_t s2 = ::send(fd, &networkSize, sizeof(uint32_t), 0);
  if (s2 < 0) {
    return;
  }

  //   Send the Payload Data(
  //   Only if there is actually data to send)
  if (payloadSize > 0) {
    // You already have the raw bytes thanks to your message.data() getter!
    ssize_t s3 = ::send(fd, message.data(), message.size(), 0);
    if (s3 < 0) {
      return;
    }
  }
}

void Server::sendToArray(const Message &message,
                         std::vector<long long> clientIDs) {
  for (auto id : clientIDs) {
    sendTo(message, id);
  }
}

// the problem here is that the function sendTo already uses our mutex
// so we would run into a deadlock- to avoid this we make a copy of the ids
void Server::sendToAll(const Message &message) {
  std::vector<long long> allIDs;
  {
    std::lock_guard<std::mutex> lock(_mutex);
    for (auto it : clientList) {
      allIDs.push_back(it.first);
    }
  }
  sendToArray(message, allIDs);
}

void Server::update() {
  std::queue<std::pair<long long, Message>> localQueue;
  {
    std::lock_guard<std::mutex> lock(_mutex);
    localQueue.swap(_inbox);
  }
  while (!localQueue.empty()) {
    long long clientID = localQueue.front().first;
    Message message = localQueue.front().second;
    localQueue.pop();
    auto it = _actions.find(message.type());
    if (it != _actions.end()) {
      it->second(clientID, message);
    }
  }
}
