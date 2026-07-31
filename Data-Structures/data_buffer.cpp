#include "data_buffer.hpp"

DataBuffer::DataBuffer() : _readPos(0) { _dataBuffer.reserve(1024); }

DataBuffer::~DataBuffer() {}