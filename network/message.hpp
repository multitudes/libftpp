#pragma once

#include "../data_structures/data_buffer.hpp"
#include <cstring> // REQUIRED for std::memcpy
#include <iostream>
#include <type_traits> // REQUIRED for std::is_trivially_copyable
#include <vector>

class Message {
private:
  int _type;
  mutable DataBuffer _payload;

public:
  using Type = int;
  Message(int type);

  size_t size() const;
  const uint8_t *data() const;
  void setBuffer(const std::vector<uint8_t> &data);
  int type() const;

  // Now, your Message just passes the work down to the DataBuffer!
  template <typename T> Message &operator<<(const T &data) {
    _payload << data;
    return *this;
  }

  template <typename T> const Message &operator>>(T &data) const {
    _payload >> data;
    return *this;
  }
};