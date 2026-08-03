#include "message.hpp"

Message::Message(int type) : _type(type), _readPos(0) { _buffer.reserve(1024); }
Message::Message() : _type(0), _readPos(0) {}
Message::~Message() {}

int Message::type() const { return _type; }

size_t Message::size() const { return _buffer.size(); }
const uint8_t *Message::data() const { return _buffer.data(); }
void Message::setBuffer(const std::vector<uint8_t> &data) { _buffer = data; }