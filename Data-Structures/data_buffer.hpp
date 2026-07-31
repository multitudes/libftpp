#pragma once
#ifndef DATA_BUFFER_HPP
#define DATA_BUFFER_HPP

#include <vector>

class DataBuffer {
private:
  std::vector<uint8_t> _dataBuffer;
  size_t _readPos;

public:
  DataBuffer();
  ~DataBuffer();

  template <typename T> DataBuffer &operator<<(const T &data) {
    const uint8_t *bytePointer = reinterpret_cast<const uint8_t *>(&data);
    _dataBuffer.insert(_dataBuffer.end(), bytePointer, bytePointer + sizeof(T));
    return *this;
  }
}
#endif