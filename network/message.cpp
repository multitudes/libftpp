#include "message.hpp"

Message::Message(int type) : _type(type), _readPos(0) { _buffer.reserve(1024); }

Message::~Message() {}

int Message::type() const { return _type; }