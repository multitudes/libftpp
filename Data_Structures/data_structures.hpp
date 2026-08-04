#pragma once

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

template <typename TType> class Pool {
public:
  class Object; // Forward declaration
  // constructors and destructors
  Pool() {}
  Pool(size_t size) { resize(size); }
  ~Pool() {}

private:
  std::vector<std::shared_ptr<Object>> pool;
  std::vector<uint8_t> _memoryBlock;
  std::vector<size_t> _availableSlots;
  size_t _maxSize = 0;

  void releaseSlot(TType *ptr) {
    size_t offsetBytes = reinterpret_cast<uint8_t *>(ptr) - _memoryBlock.data();
    size_t index = offsetBytes / sizeof(TType);
    _availableSlots.push_back(index);
  }

public:
  // Allocates a certain number of TType objects within the Pool - this is
  // done only if the pool is empty
  void resize(const size_t &numberOfObjectStored) {
    if (numberOfObjectStored == _maxSize) {
      // the size isn't changing, just do nothing
      std::cout << ">>> The size " << _maxSize << " isn't changing!"
                << std::endl;
      return;
    }
    if ((_maxSize - _availableSlots.size()) > 0) {
      std::cout << ">>> Cannot resize pool while objects are currently in use!"
                << std::endl;
      return;
    }
    _maxSize = numberOfObjectStored;
    _memoryBlock.resize(_maxSize * sizeof(TType));
    _availableSlots.clear();
    for (size_t i = _maxSize; i > 0; --i) {
      _availableSlots.push_back(i - 1);
    }
  }

  // need the variadic TArgs which will contain what I pass to the Object
  // constructor!
  template <typename... TArgs> Object acquire(TArgs &&...p_args) {
    if (_availableSlots.empty()) {
      std::cout << "Pool is full! Cannot acquire more objects." << std::endl;
      // Return an empty wrapper instead of throwing
      return Object(nullptr, nullptr);
    }

    size_t index = _availableSlots.back();
    _availableSlots.pop_back();

    // Calculate the exact memory address
    uint8_t *rawAddress = _memoryBlock.data() + (index * sizeof(TType));
    TType *obj_ptr = new (rawAddress) TType(std::forward<TArgs>(p_args)...);

    return Object(obj_ptr, this);
  }
};

template <typename TType> class Pool<TType>::Object {
private:
  TType *_ptr;
  Pool *_pool;

public:
  Object(TType *ptr = nullptr, Pool *pool = nullptr) : _ptr(ptr), _pool(pool) {}
  ~Object() {
    std::cout << "Entering Pool::Object destructor " << std::endl;
    if (_ptr) {
      // Tell the pool this specific index is free again
      if (_pool) {
        std::cout << "Releasing the object from the Pool" << std::endl;
        std::cout << "Pointer" << _ptr << std::endl;
        _pool->releaseSlot(_ptr);
      }
      std::cout << "calling the object destructor." << std::endl;
      _ptr->~TType();
    }
  }
  // dont allow copies just for safety!
  // Object(const Object &other) = delete;
  // Object &operator=(const Object &other) = delete;

  // Allows the user to say: if (myObject) { ... }
  // need explicit to avoid unwanted behavior see read me
  explicit operator bool() const { return _ptr != nullptr; }

  TType *operator->() { return _ptr; }
};
