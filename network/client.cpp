#include "client.hpp"
#include "message.hpp"
#include <arpa/inet.h>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <netdb.h>
#include <sys/_endian.h>
#include <sys/socket.h>
#include <unistd.h>

Client::Client() : _socketFd(-1), _isConnected(false) {}

Client::~Client() { disconnect(); }

void Client::connect(const std::string &address, const size_t &port) {
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 100000;
  _socketFd = socket(AF_INET, SOCK_STREAM, 0);
  if (_socketFd < 0) {
    std::cerr << "Socket creation error\n";
    return;
  }

  setsockopt(_socketFd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));

  struct sockaddr_in serv_addr;
  serv_addr.sin_family = AF_INET; // set family to ipv4
  serv_addr.sin_port = htons(port);

  // does not understand localhost only numbers
  //   if (inet_pton(AF_INET, address.c_str(), &serv_addr.sin_addr) <= 0) {
  //     std::cerr << "Invalid address / Address not supported\n";
  //     return;
  //   }

  struct hostent *server = gethostbyname(address.c_str());
  if (server == nullptr) {
    std::cerr << "Error, no such host: " << address << "\n";
    return;
  }
  // Copy the resolved IP address into your sockaddr_in struct
  std::memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr,
              server->h_length);

  if (::connect(_socketFd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <
      0) {
    std::cerr << "Connection Failed\n";
    return;
  }
  _isConnected = true;
  _listenerThread = std::unique_ptr<Thread>(
      new Thread("Client", [this]() { this->_listenLoop(); }));
  _listenerThread->start();
  std::cout << "Connected successfully to " << address << ":" << port << "\n";
}

// refactred to keep the connect function clean
void Client::_listenLoop() {
  while (_isConnected) {
    int type;
    ssize_t bytes = ::recv(_socketFd, &type, sizeof(int), 0);

    if (bytes < 0)
      continue;
    if (bytes == 0) {
      _isConnected = false;
      break;
    }
    type = ntohl(type);

    // Read the Payload Size (4 bytes) - this is not in the subj but cannot
    // be done without. like the content_length in http...
    uint32_t payloadSize = 0;
    bytes = ::recv(_socketFd, &payloadSize, sizeof(uint32_t), 0);

    if (bytes <= 0) {
      _isConnected = false;
      break;
    }
    payloadSize = ntohl(payloadSize);

    Message msg(type);

    std::vector<uint8_t> tempBuffer(payloadSize);
    size_t totalRead = 0;

    while (totalRead < payloadSize) {
      // recv returns a ssize_t - important!
      ssize_t r = ::recv(_socketFd, tempBuffer.data() + totalRead,
                         payloadSize - totalRead, 0);

      if (r > 0) {
        totalRead += r;
      } else if (r == 0) {
        _isConnected = false;
        break;
      }
    }
    msg.setBuffer(tempBuffer);
    std::lock_guard<std::mutex> lock(_mutex);
    _inbox.push(msg);
  }
}

void Client::disconnect() {
  _isConnected = false;

  // 1. Only close the socket if it was actually opened
  if (_socketFd != -1) {
    close(_socketFd);
    _socketFd = -1; // Reset to our constructor's default state
  }

  // 2. Only stop the thread if it was actually created
  if (_listenerThread) {
    _listenerThread->stop();
    _listenerThread.reset(); // Safely destroys the thread object
  }
}

void Client::defineAction(
    const Message::Type &messageType,
    const std::function<void(const Message &msg)> &action) {
  _actions[messageType] = action;
}

void Client::send(const Message &message) {
  if (!_isConnected || _socketFd == -1)
    return;
  //   Send the Message Type
  int networkType = htonl(message.type());
  ssize_t s1 = ::send(_socketFd, &networkType, sizeof(int), 0);
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
  ssize_t s2 = ::send(_socketFd, &networkSize, sizeof(uint32_t), 0);
  if (s2 < 0) {
    return;
  }

  //   Send the Payload Data(
  //   Only if there is actually data to send) if (payloadSize > 0)
  if (payloadSize > 0) {
    // You already have the raw bytes thanks to your message.data() getter!
    ssize_t s3 = ::send(_socketFd, message.data(), message.size(), 0);
    if (s3 < 0) {
      return;
    }
  }
}

void Client::update() {
  std::queue<Message> localQueue;
  {
    std::lock_guard<std::mutex> lock(_mutex);
    localQueue.swap(_inbox);
  }
  // now I can work on the localqueue
  while (!localQueue.empty()) {
    Message msg = localQueue.front();
    localQueue.pop();
    int type = msg.type();
    auto it = _actions.find(type);
    if (it != _actions.end()) {
      it->second(msg);
    } else {
      // Optional: Log that an unknown message type was received
      std::cout << "[Client] Unhandled message type: " << msg.type() << "\n";
    }
  }
}