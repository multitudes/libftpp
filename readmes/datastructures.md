# Data Structures

Here is a breakdown of the data structures I implemented for this project, specifically focusing on the Object Pool and the binary serialization DataBuffer.

## 1. Object Pool

Dynamically allocating (using `new`) and deallocating (using `delete`) memory during runtime is computationally expensive. To solve this, I implemented the Object Pool design pattern.

Instead of creating a new object from scratch every time we need one, we pre-allocate a large batch (a "pool") of memory upfront. When we need an object, we "acquire" an unused chunk of this memory and construct our object there. When we are done, we release the memory back to the pool to be reused.

Crucially, when the object is released, we call the destructor of the `TType` object to clean up its state, but we **do not deallocate the memory**. The space stays reserved for the next time we need it.

Here are the core methods implemented:

```cpp
// Resizes the pool (only possible if empty!)
void resize(const size_t& numberOfObjectStored);

// Allocates a certain number of TType objects within the allocated memory
template<typename ... TArgs> 
Pool::Object<TType> acquire(TArgs&&... p_args);

// Creates a Pool::Object containing a pre-allocated size for n objects
Pool(size_t size);

// Returns the pointer stored within the Pool::Object
TType* operator->();

```

### Implementing RAII

Every request and release of an object must be handled safely, so I used the RAII (Resource Acquisition Is Initialization) idiom. The user doesn't get a raw pointer; they get a `Pool::Object` wrapper. When that wrapper goes out of scope, its destructor automatically handles returning the resource to the pool.

### Variadic Templates and Forwarding References

To allow the `acquire` function to accept any number of constructor arguments for our objects, I used Variadic Templates. When passing these arguments, there are three distinct approaches:

1. **Pass by Value:** Makes a full copy of everything. (Too slow for large objects).
2. **Pass by Const Reference:** Fast and safe, but breaks if the user passes a temporary value (an rvalue) like a raw number.
3. **Pass by Forwarding Reference (`&&`):** The modern C++ solution.

When we attach `&&` to a template parameter, it becomes a "Forwarding Reference" (or Universal Reference). The compiler looks at what the user passed and adapts: if it's a standard variable, it collapses into a normal reference (`&`). If it's a temporary value, it remains an rvalue reference (`&&`).

To achieve "Perfect Forwarding", we pair this with `std::forward` and placement `new`:

```cpp
#include <utility> // Required for std::forward

template <typename... TArgs> 
Object acquire(TArgs&&... p_args) {
    // 1. Find the raw memory address in the pool
    void* memory_address = ...;

    // 2. Construct the object using placement new and perfect forwarding
    TType* constructed_object = new (memory_address) TType(std::forward<TArgs>(p_args)...);

    // 3. Return the Object wrapper
    return Object(constructed_object, this);
}

```

### Operator Overloading: `->`

When `acquire()` returns our `Pool::Object`, the user receives a wrapper class. If they want to call a method on their object, typing `myObject->printPosition()` would normally confuse the compiler.

To fix this, I overloaded the arrow operator.

```cpp
TType* operator->() { return _ptr; }

```

This tells the compiler: *"If anyone uses the `->` operator on my wrapper, automatically reach inside and give them the raw pointer."* This technique is exactly how standard smart pointers (`std::unique_ptr`) work under the hood, allowing the user's code to feel completely natural:

```cpp
// The compiler automatically translates this:
myObject->printPosition();

// Into this under the hood:
myObject.operator->()->printPosition();

```

### Using `explicit` for `operator bool()`

In C++, compilers love to perform implicit conversions to make code compile. If we wrote `operator bool() const` without the `explicit` keyword, our wrapper object could secretly be treated as a `0` or `1` anywhere in the program:

```cpp
auto myObject = particlePool.acquire("Alpha", 1, 2, 3);

// Perfectly valid C++ without 'explicit'. myObject evaluates to 'true' (1).
int mathResult = myObject + 100; // Becomes 101!

```

To prevent these massive logical bugs, I added `explicit`:

```cpp
explicit operator bool() const { return _ptr != nullptr; }

```

Now, the compiler is only allowed to convert this wrapper to a boolean in strict `if` statements. Any bad math code will immediately throw a compile-time error.

---

## 2. DataBuffer: A Binary Serialization Buffer

The project required storing objects in byte format using C++ stream operators. Essentially, I built a class that acts like `std::cout`, but instead of printing text to a screen, it converts variables into raw binary bytes and pushes them into a `std::vector<uint8_t>`.

### Operator Chaining

To allow multiple variables to be written on a single line, the `<<` operator must return a reference to the buffer itself.

```cpp
template <typename T> 
DataBuffer& operator<<(const T& data) {
    const uint8_t* bytePointer = reinterpret_cast<const uint8_t*>(&data);
    _dataBuffer.insert(_dataBuffer.end(), bytePointer, bytePointer + sizeof(T));
    return *this;
}

```

Because it returns `*this`, we can chain operations naturally:

```cpp
myBuffer << playerHealth << playerX << isPoisoned;

```

### The Data Flow (`<<` and `>>`)

The easiest way to understand these operators is to view them as visual arrows showing where the data flows:

* `myBuffer << obj1;` (Data from `obj1` flows **into** the buffer).
* `myBuffer >> obj1;` (Data flows out of the buffer and **into** `obj1`).

### Using the Buffer (FIFO)

Writing to the buffer doesn't require a custom playhead; `std::vector::insert` simply tacks new bytes onto the end. However, **reading** requires a custom `_readPos` variable because we aren't destroying the tape as we read it—we need to remember where we left off.

Because the buffer is just raw bytes, it forgets the *types* of the variables it stores. We must read the data back in a strict First-In, First-Out (FIFO) order, using the exact same types we wrote:

```cpp
int healthOut;
float xOut;
bool poisonOut;

// We must read in the exact order we wrote: int, float, bool
myBuffer >> healthOut >> xOut >> poisonOut; 

```

### Const Correctness

Applying `const` correctly is crucial for this class. There are two different applications of it here:

```cpp
size_t size() const { return _buffer.size(); }

const std::vector<uint8_t>& getBuffer() const { return _buffer; }

```

1. **Method-level `const`:** Putting `const` at the end of a method (like `size() const`) is a promise to the compiler that the function will not modify any of the class's internal variables. This is mandatory if you ever pass the buffer to another function by `const` reference.
2. **Return-type `const`:** In `const std::vector<uint8_t>&`, the `const` protects the reference we are handing back to the user. Returning by reference is fast, but without `const`, a user could grab our internal array and accidentally call `.clear()` on it, destroying our data.

### Serializing Strings and `static_assert`

I had to write a custom overload for `std::string`. Standard strings are not "trivially copyable" because they often contain pointer references to memory on the heap. Serializing a raw pointer address is useless—it just becomes a dangling pointer upon deserialization.

To protect the buffer from developers accidentally passing complex objects, I added a compile-time check using `static_assert`:

```cpp
static_assert(std::is_trivially_copyable<T>::value,
              "DataBuffer ERROR: Type is not trivially copyable! You must write a custom overload.");

```

Unlike a standard `assert()` which checks at runtime, `static_assert` evaluates before the executable is even built, throwing an immediate error if the condition fails.

### Include What You Use (IWYU)

Finally, I followed the modern C++ "Include What You Use" philosophy. My `libftpp.hpp` acts as an Umbrella Header—it simply includes all the tiny sub-headers so the user only has to include one file. To satisfy strict linters, I used the export pragma:

```cpp
#include "data_structures/data_buffer.hpp" // IWYU pragma: export

```
