#pragma once

#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <string>
#include <type_traits>
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
    static_assert(std::is_trivially_copyable<T>::value,
                  "DataBuffer ERROR: Type is not trivially copyable! You must "
                  "write a custom overload.");
    const uint8_t *bytePointer = reinterpret_cast<const uint8_t *>(&data);
    _buffer.insert(_buffer.end(), bytePointer, bytePointer + sizeof(T));
    return *this;
  }

  // Deserialization (Reading FROM the buffer)
  template <typename T> DataBuffer &operator>>(T &data) {
    static_assert(std::is_trivially_copyable<T>::value,
                  "DataBuffer ERROR: Type is not trivially copyable! You must "
                  "write a custom overload.");
    if (_readPos + sizeof(T) > _buffer.size()) {
      throw std::runtime_error("not enough bytes to read!");
    }
    std::memcpy(&data, _buffer.data() + _readPos, sizeof(T));
    _readPos += sizeof(T);
    return *this;
  }

  // --- STRING OVERLOADS (For dynamic memory) ---

  // Serialization (Writing TO the buffer)
  DataBuffer &operator<<(const std::string &data);

  // Deserialization (Reading FROM the buffer)
  DataBuffer &operator>>(std::string &data);

  // not in the subject but nice to have
  void clear();
  size_t size() const;
  const std::vector<uint8_t> &getBuffer() const;
  void setBuffer(const std::vector<uint8_t> &newBuffer);
};
