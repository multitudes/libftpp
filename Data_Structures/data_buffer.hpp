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
  DataBuffer &operator<<(const std::string &data) {
    size_t len = data.length();
    *this << len; // Use our own template to write the size_t length

    // Insert the actual characters
    _buffer.insert(_buffer.end(), data.begin(), data.end());
    return *this;
  }

  // Deserialization (Reading FROM the buffer)
  DataBuffer &operator>>(std::string &data) {
    size_t len;
    *this >> len; // Use our own template to read the size_t length

    if (_readPos + len > _buffer.size()) {
      throw std::runtime_error("not enough bytes to read!");
    }

    // Assign the characters directly into the string
    data.assign(reinterpret_cast<const char *>(_buffer.data() + _readPos), len);
    _readPos += len;

    return *this;
  }

  // not in the subject but nice to have
  void clear();
  size_t size() const;
  const std::vector<uint8_t> &getBuffer() const;
};
