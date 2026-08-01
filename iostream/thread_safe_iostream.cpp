#include "thread_safe_iostream.hpp"
#include <mutex>

// allocate the static mutex
std::mutex ThreadSafeIOStream::_io_mutex;

// Create the global thread_local instance requested by the subject.
// Every thread that includes the header will automatically get its own unique
// copy of this!
thread_local ThreadSafeIOStream threadSafeCout;