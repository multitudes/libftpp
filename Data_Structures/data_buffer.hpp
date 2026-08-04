#pragma once

#include <cstddef>
#include <cstring>

#include <iostream>
#include <vector>

class DataBuffer {
private:
  std::vector<uint8_t> _buffer;
  size_t _readPos;

public:
  DataBuffer();
  ~DataBuffer();

  // Serialization (Writing TO the buffer)
  template <typename T> DataBuffer &operator<<(const T &data) {
    const uint8_t *bytePointer = reinterpret_cast<const uint8_t *>(&data);
    _buffer.insert(_buffer.end(), bytePointer, bytePointer + sizeof(T));
    return *this;
  }

  // Deserialization (Reading FROM the buffer)
  template <typename T> DataBuffer &operator>>(T &data) {
    if (_readPos + sizeof(T) > _buffer.size()) {
      std::cout << "not enough bytes to read!" << std::endl;
      return *this;
    }
    std::memcpy(&data, _buffer.data() + _readPos, sizeof(T));
    _readPos += sizeof(T);
    return *this;
  }

  // not in the subject but nice to have
  void clear();
  size_t size() const;
  const std::vector<uint8_t> &getBuffer() const;
};
