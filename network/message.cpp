#include "message.hpp"

Message::Message(int type) : _type(type) {}

int Message::type() const { return _type; }

size_t Message::size() const { return _payload.size(); }

const uint8_t *Message::data() const { return _payload.getBuffer().data(); }

void Message::setBuffer(const std::vector<uint8_t> &data) {
  _payload.setBuffer(data); // Pass it down to the DataBuffer
}