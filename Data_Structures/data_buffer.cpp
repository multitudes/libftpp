#include "data_buffer.hpp"
#include <cstddef>
#include <cstdint>

DataBuffer::DataBuffer() : _readPos(0) { _buffer.reserve(1024); }

DataBuffer::~DataBuffer() {}

void DataBuffer::clear() {
  _buffer.clear();
  _readPos = 0;
}

size_t DataBuffer::size() const { return _buffer.size(); }

const std::vector<uint8_t> &DataBuffer::getBuffer() const { return _buffer; }

// --- STRING OVERLOADS (For dynamic memory) ---

// Serialization (Writing TO the buffer)
// note the str is const - i do not modify the orig object to serialize
// for strings I need to pass the lenght of the string
DataBuffer &DataBuffer::operator<<(const std::string &data) {
  size_t len = data.length();
  *this << len;
  _buffer.insert(_buffer.end(), data.begin(), data.end());
  return *this;
}

// Deserialization (Reading FROM the buffer)
// no const keyword because I modify the buffer
// for strings I need to pass the lenght of the string
DataBuffer &DataBuffer::operator>>(std::string &data) {
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
