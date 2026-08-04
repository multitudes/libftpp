#pragma once

#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <cstring> // REQUIRED for std::memcpy
#include <iostream>
#include <type_traits> // REQUIRED for std::is_trivially_copyable
#include <vector>

class Message {
private:
  int _type;
  std::vector<uint8_t> _buffer;
  mutable size_t _readPos;

public:
  // to make compatible with the subject
  using Type = int;
  Message();
  Message(int type);
  ~Message();

  int type() const;

  // needed by class Client to facilitate the receiving and sending of data
  size_t size() const;
  const uint8_t *data() const;
  void setBuffer(const std::vector<uint8_t> &data);

  // Serialization (Writing TO the buffer)
  template <typename T> Message &operator<<(const T &data) {
    // if i try to send complex types like string the system would crash
    // because it would send pointers in the message which would become invalid
    // over the network
    static_assert(std::is_trivially_copyable<T>::value,
                  "Data must be trivially copyable");
    const uint8_t *bytePointer = reinterpret_cast<const uint8_t *>(&data);
    _buffer.insert(_buffer.end(), bytePointer, bytePointer + sizeof(T));
    return *this;
  }

  // Deserialization (Reading FROM the buffer)
  template <typename T> const Message &operator>>(T &data) const {
    // prevent crashes - see above remark
    static_assert(std::is_trivially_copyable<T>::value,
                  "Data must be trivially copyable");
    if (_readPos + sizeof(T) > _buffer.size()) {
      std::cout << "not enough bytes to read!" << std::endl;
      return *this;
    }
    std::memcpy(&data, _buffer.data() + _readPos, sizeof(T));
    _readPos += sizeof(T);
    return *this;
  }
};

#endif