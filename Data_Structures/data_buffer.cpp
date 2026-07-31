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