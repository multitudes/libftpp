# 42-libftpp

## What is a Pool?

In software engineering, an Object Pool is a creational design pattern used to manage performance and memory.
Dynamically allocating (using new) and deallocating (using delete) memory during runtime is computationally expensive. If you are building a system that frequently creates and destroys small objects—like bullets in a video game, or network packets—that overhead will slow your program down.
A Pool solves this by pre-allocating a large batch (a "pool") of memory for these objects upfront. According to the subject, your Pool class "manages a collection of reusable templated TType objects". Instead of creating a new object from scratch, you "acquire" an existing, unused chunk of memory from the pool, construct your object there, and when you are done, you give the memory back to the pool to be reused.  
Crucially, when the object is released back to the pool, the subject requires "calling the destructor of the TType object but without deallocating the memory". The memory stays reserved for the next time you need it.  

- Variadic Templates and Perfect Forwarding: The subject asks to implement the acquire method 

```cpp
void resize(const size_t& numberOfObjectStored):

// Allocates a certain number of TType objects withing the Pool.
template<typename ... TArgs> Pool::Object<TType>
acquire(TArgs&& p_args): 

// Creates a Pool::Object containing a pre-allocated object, 
// using the constructor with parameters as defined by TArgs definition.

// Pool::Object :
-TType* operator -> (): 
// Returns the pointer stored within the Pool::Object.
```

RAII (Resource Acquisition Is Initialization): The subject states that "Every requests and releases of a pre-allocated objects must be handled by Pool::Object, not by the user!". The Pool::Object acts as a custom smart wrapper. When a user requests an object, they get a Pool::Object. When that Pool::Object goes out of scope, its destructor should automatically handle returning the resource to the pool.

TType is the "What": This is the type of the final object you are storing in the pool (e.g., a Player, a Bullet, or a std::string).  
TArgs are the "Ingredients": This represents the arguments passed into the constructor of TType.  


## NEW in CPP11

You have spotted one of the most powerful—and notoriously confusing—features introduced in C++11!

While `&&` normally means an **rvalue reference** (which is used for moving data rather than copying it), when it is attached to a deduced template type like `TArgs`, it transforms into something entirely different. It becomes a **Forwarding Reference** (also famously called a "Universal Reference").

Here is exactly why you need it in your `acquire` function.

### The Problem it Solves: Perfect Forwarding

Your `acquire` function is a middleman. Its only job is to take arguments from the user and hand them *exactly as they are* to the `TType` constructor.

Without `&&`, you would run into a wall depending on how the user calls your function:

1. **Pass by Value (`TArgs... p_args`)**: This would make a full copy of every argument. If the user passes a massive data structure, copying it just to hand it to the constructor is a huge performance hit.
2. **Pass by Reference (`TArgs&... p_args`)**: This avoids copies, but it completely breaks if the user passes a temporary value (an rvalue). You wouldn't be able to do `pool.acquire(5)`, because you can't bind a standard reference to a raw number.
3. **Pass by Const Reference (`const TArgs&... p_args`)**: This lets you pass temporaries, but it strips away the ability to use "move semantics." If the user wants to efficiently move data into the object, the `const` kills that optimization.

### The Solution: `TArgs&&`

When the compiler sees `TArgs&&` in a template, it uses a trick called "reference collapsing." It looks at what the user actually passed and adapts perfectly:

* If the user passes a normal, named variable (an lvalue), `TArgs&&` collapses into a normal reference (`&`).
* If the user passes a temporary value (an rvalue, like a raw `5`), `TArgs&&` remains an rvalue reference (`&&`).

It acts as a perfect mold of whatever was passed to it.

### How You Actually Use It

To make this "Perfect Forwarding" actually work inside your function body, you must pair `&&` with a standard library tool called `std::forward`.

When you get to writing the placement `new` inside your `acquire` method, it will look exactly like this:

```cpp
#include <utility> // Required for std::forward

// ...

template <typename... TArgs> 
Object acquire(TArgs&&... p_args) {
    // 1. Find your raw memory address
    void* memory_address = /* your logic here */;

    // 2. Construct the object using placement new and perfect forwarding
    TType* constructed_object = new (memory_address) TType(std::forward<TArgs>(p_args)...);

    // 3. Return the Object wrapper
    // ...
}

```

By using `TArgs&&` in the parameter list and `std::forward<TArgs>(p_args)...` in the function body, you guarantee that the arguments arrive at the constructor in the exact same state (copyable, movable, const, or non-const) as when the user passed them.

Awesome, I'm glad it compiled and works!

The line `TType *operator->() { return _ptr; }` is a concept in C++ called **Operator Overloading**. It is what makes your custom `Pool::Object` behave like a real, built-in pointer.

Here is exactly why you need it and how it works.

### The Problem: The Wrapper Barrier

When `acquire()` returns a `Pool::Object`, the user receives a wrapper, not the raw `Particle*` (or whatever `TType` is).

If the user wants to call a method on their particle, like `printPosition()`, the compiler will get confused if they just type `myObject->printPosition()`. The compiler sees that `myObject` is a `Pool::Object` class, not a pointer, and it doesn't have a `printPosition()` method inside it.

Without overloading the arrow operator, you would have to force the user to do something ugly and clunky like this:

```cpp
// If we used a normal getter method like getPtr()
myObject.getPtr()->printPosition(); 

```

### The Solution: `operator->()`

By defining `TType *operator->()`, you are teaching the C++ compiler a new trick. You are saying: *"If anyone ever uses the `->` operator directly on my `Pool::Object`, automatically reach inside and give them the raw `_ptr` instead."*

Because of this overload, the user can write code that feels completely natural, exactly as if they were holding a real pointer:

```cpp
// The compiler automatically translates this:
myObject->printPosition();

// Into this under the hood:
myObject.operator->()->printPosition();

```

### Required by the Subject

This feature is actually a strict requirement for the project. The subject explicitly dictates that your `Pool::Object` must include `- TType* operator -> (): Returns the pointer stored withing the Pool:: Object.`.

This technique is exactly how standard library smart pointers (like `std::unique_ptr` and `std::shared_ptr`) work under the hood. You have essentially built your own custom smart pointer!


### Why `explicit` is critical for `operator bool()`

In C++, compilers love to automatically convert types (called **implicit conversion**) to try and make your code compile.

If you just write `operator bool() const` without the `explicit` keyword, your object can be secretly treated as a `bool` (which is essentially a `0` or `1`) anywhere in your program.

Here is the nightmare scenario of what happens **without** `explicit`:

```cpp
auto myObject = particlePool.acquire("Alpha", 1, 2, 3);

// This is perfectly valid C++ without 'explicit'.
// myObject evaluates to 'true' (1), so mathResult becomes 101.
int mathResult = myObject + 100; 

// Or if a function expects an integer, you could accidentally pass the object!
someFunctionExpectingInt(myObject); 

```

The compiler won't warn you; it will just silently convert your custom `Pool::Object` into a `1` and continue running, creating a massive logical bug.

### What `explicit` fixes

By adding `explicit`:

```cpp
explicit operator bool() const { return _ptr != nullptr; }

```

You are telling the compiler: *"You are only allowed to convert this to a boolean in strict, explicit boolean contexts."*

With `explicit`, the bad math code (`myObject + 100`) will immediately throw a compile-time error. But it will still perfectly allow the safe, logical checks you actually want:

```cpp
if (myObject) { ... }           // Allowed!
while (myObject) { ... }        // Allowed!
if (!myObject || otherThing)    // Allowed!

```


## DataBuffer, a binary serialization buffer.

The subject says "storing objects in byte format" and "Use C++ stream operators", it is asking you to build a class that acts like std::cout or std::cin, but instead of printing text to a screen, it converts variables into raw binary bytes and pushes them into a vector.

### returning references for chaining
```cpp
template <typename T> DataBuffer &operator<<(const T &data) {
const uint8_t *bytePointer = reinterpret_cast<const uint8_t *>(&data);
_dataBuffer.insert(_dataBuffer.end(), bytePointer, bytePointer + sizeof(T));
return *this;
```

Why am I expecting a reference as return? Is this legal?

### 1. What is `*this`?

When you call `myBuffer << playerHealth;`, you are calling the `operator<<` function *on* an existing object (`myBuffer`).

- `this` is a hidden pointer that C++ automatically passes into the function, pointing to `myBuffer` in memory.
- `*this` dereferences that pointer, meaning it represents the actual, physical `myBuffer` object itself.

Because `myBuffer` exists outside the function (usually in `main()` or another class), it does not get destroyed when the function ends. Therefore, returning a reference to it (`DataBuffer&`) is 100% safe and legal!

Why do we return `DataBuffer&`?

We return a reference to the object so that we can do something called 'Operator Chaining'.

But because `operator<<` returns a reference to the very buffer it just modified, you can chain them together on a single line:

```cpp
myBuffer << x << y << z;

```

This is the exact same way standard C++ streams like `std::cout << "Hello " << "World!";` work.

### Using the buffer

- **Writing** doesn't need a custom playhead because `std::vector::insert` always just tacks the new bytes onto the very end of the vector. The vector's `.size()` naturally acts as the "write head."
- **Reading** requires `_readPos` because we aren't destroying the tape as we read it; we need to remember where we left off.

Because your buffer is a `std::vector<uint8_t>`, it has no idea what an `int` or a `float` or a `char` is. It only sees raw, meaningless bytes. This means it can mix and match completely different data types in the exact same buffer.

You can do this:

```cpp
DataBuffer myBuffer;

int playerHealth = 100;
float playerX = 45.5f;
bool isPoisoned = true;

// Write all three DIFFERENT types into the same buffer!
myBuffer << playerHealth << playerX << isPoisoned;

```

Under the hood, `myBuffer` just sees: 4 bytes (int) + 4 bytes (float) + 1 byte (bool) = 9 total bytes of raw data.

However, because the buffer forgets the *types* of the variables it stores, you must read the data back in the **exact same order** and with the **exact same types** that you wrote it. It is a strict First-In, First-Out (FIFO) system.

If we read the mixed buffer from the example above, we must do it like this:

```cpp
int healthOut;
float xOut;
bool poisonOut;

// We must read in the exact order: int, float, bool
myBuffer >> healthOut >> xOut >> poisonOut; 

```


------
ok right. why those two are const?
size_t DataBuffer::size() const { return _buffer.size(); }

const std::vector<uint8_t> &DataBuffer : getBuffer() const {
    return _buffer;
}

You just stumbled onto one of the most important best practices in C++: **Const Correctness**.

There are actually two completely different types of `const` happening in those lines. Let's break them down, because understanding this will make your C++ code infinitely safer and more professional.

### 1. The `const` at the end (The "Read-Only" Promise)

Look at the end of both function signatures:

* `size_t size() const`
* `... getBuffer() const`

Putting `const` at the very end of a class method is a promise to the compiler: **"I swear this function will not change any of the class's variables."**

Because `size()` only reads the size, and `getBuffer()` only reads the vector, they do not modify `_buffer` or `_readPos`.

**Why do we care?**
If you pass your buffer into another function, you usually pass it as a constant reference to prevent accidental changes.

```cpp
void sendOverNetwork(const DataBuffer& bufferToSend) {
    // Because bufferToSend is const, the compiler ONLY lets you call 
    // methods that also have the 'const' promise at the end.
    
    size_t s = bufferToSend.size(); // Allowed!
    bufferToSend.clear();           // Compiler error! clear() isn't const.
}

```

If you forgot to put `const` at the end of `size()`, the compiler would refuse to let you check the size inside `sendOverNetwork`, even though reading the size is harmless!

---

### 2. The `const` at the beginning (The "Look But Don't Touch" Shield)

Now look at the return type of the second function:

* `const std::vector<uint8_t> &`

This `const` applies to what you are **handing back to the user**.

You are returning a reference (`&`) to your internal `_buffer`. Returning by reference is extremely fast because it doesn't copy the massive array of bytes. However, giving someone a reference to your internal variable is dangerous.

**Without `const`:**
If you just returned `std::vector<uint8_t>&`, the user could do this:

```cpp
// The user grabs your internal vector and destroys it!
myBuffer.getBuffer().clear(); 

```

Your `DataBuffer` wouldn't know the vector was cleared, and its `_readPos` would now be pointing to memory that doesn't exist. Instant crash.

**With `const`:**
By returning a `const ... &`, you are putting the vector behind museum glass. You are telling the user: *"You can look at the bytes, you can copy them, you can send them over a network, but the compiler will throw a massive error if you try to modify them."*


## The "Include What You Use" Rule (IWYU)

Modern C++ linters follow a strict philosophy called Include What You Use. This rule states that if you use a class (like DataBuffer) in your main.cpp, you should include the exact, specific file where DataBuffer is defined.

Your libftpp.hpp file is what C++ developers call an Umbrella Header. It acts like a giant folder that just includes all your other tiny headers (data_structures.hpp, data_buffer.hpp, etc.) so the user only has to include one thing.

When you use DataBuffer in main.cpp, clangd looks at your code and says:
"Wait a minute... DataBuffer actually lives in data_buffer.hpp. You included libftpp.hpp, but you aren't using anything defined directly inside it! You should include data_buffer.hpp instead."

## Memento

It is a design pattern of the gang of four. Allows to take snapshots of an object. 
The class Memento offers as public method the same and load function which will use a DataBuffer as a Snapshot- we already implemented a databuffer! so we gonna use it 
```cpp
using Snapshot = DataBuffer;
```

The full class also will be virtual. Nered a virtual destructor and also the methods (private) to be implemented in the children class also made virtual.

```cpp
#pragma once

#include "data_buffer.hpp"

class Memento {
public:
    // 1. The type alias we just talked about!
    using Snapshot = DataBuffer;

    // 2. Virtual destructor is mandatory for classes meant to be inherited!
    virtual ~Memento() = default;

    // 3. The public interface for the user
    Snapshot save() const; 
    void load(const Snapshot& state);

private:
    // 4. The "Pure Virtual" methods. 
    // The '= 0' means Memento has no code for these; the child MUST provide it.
    virtual void _saveToSnapshot(Snapshot& snapshot) const = 0;
    virtual void _loadFromSnapshot(Snapshot& snapshot) = 0;
};
```
think about how the parent Memento class actually executes the saving. When you call save(), Memento needs to turn around and call _saveToSnapshot() on the child class.

Since Memento doesn't know what child class is inheriting from it (it could be a Player, a Particle, or a Car), what C++ keyword do we need to attach to those private _saveToSnapshot and _loadFromSnapshot methods so that the parent can trigger the child's specific version of them?

the 42 subject explicitly dropped the hint ("I wonder if there is a friendly way to do it...")When you (or anyone else) write a class that inherits Memento, it must look like this:

```cpp
class Player : public Memento {
private:
    // 1. The explicit invite! This grants Memento access to the private methods below.
    friend class Memento; 

    // 2. The private data
    int _health;
    float _x, _y;

    // 3. The private implementations of the virtual contract
    void _saveToSnapshot(Memento::Snapshot& snapshot) const override {
        snapshot << _health << _x << _y;
    }

    void _loadFromSnapshot(Memento::Snapshot& snapshot) override {
        snapshot >> _health >> _x >> _y;
    }
};
```
