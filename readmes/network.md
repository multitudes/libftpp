# Network

## Message

For the `Message` class, which handles communication between the client and server, every message needs to be labeled so the receiver knows what to do with it.

* **The Constructor `Message(int type)`:** Tags the message upon creation (e.g., `1` for "Login Request", `2` for "Player Movement").
* **The Method `int type()`:** Allows the server to read this tag upon receipt.

To make inserting and extracting data intuitive, I overloaded the `<<` operator to push data *into* the payload and `>>` to pull data *out* of it. For example:
`myMessage << playerX << playerY << playerHealth;`

Because the subject asks for **templated** operator overloads, we only need to write exactly one `<<` function and one `>>` function. The compiler will automatically adapt it to accept integers, floats, doubles, or even custom structures, figuring out the byte size automatically.

### The Composition Approach

Instead of rewriting memory management logic, I reused the `DataBuffer` class I had already coded for a previous exercise. By including `DataBuffer` as a private member inside `Message`, the `Message` class simply passes the data straight through to the buffer.

```cpp
template <typename T> 
Message& operator<<(const T& data) {
    _buffer << data; // Passes it to the DataBuffer object
    return *this;
}

```

## Transferring Queues safely

When moving messages around, it is important to remember that `std::queue::pop()` returns `void` for exception safety. If `pop()` returned the element by value and the copy constructor threw an exception, the element would be permanently lost.

Here are the three ways to handle moving queue data, depending on the need:

### 1. `std::swap` or `std::move` (O(1) time)

This is the standard approach for thread-safe queues to minimize mutex lock time. We just swap the internal pointers instantly. Because `localQueue` starts out empty, swapping means `localQueue` gets all the messages, and `_inbox` takes on the empty state.

```cpp
// Option A: Swap (Recommended)
std::queue<Message> localQueue;
localQueue.swap(_inbox); 
// _inbox is now empty, localQueue has everything.

// Option B: Move constructor
std::queue<Message> localQueue(std::move(_inbox));

```

### 2. The True Copy (O(N) time)

If you need to keep the contents of `_inbox` intact and literally copy everything, use the copy constructor. No loop required.

```cpp
std::queue<Message> localQueue = _inbox;

```

### 3. With a `while` loop

If we need to filter messages as we transfer them, we have to look at the `front()` element before we `pop()` it. We use `std::move` to avoid unnecessary copying.

```cpp
std::queue<Message> localQueue;

while (!_inbox.empty()) {
    // 1. Get the element at the front (use std::move to avoid copying)
    localQueue.push(std::move(_inbox.front())); 
    
    // 2. Remove the element from the original queue
    _inbox.pop(); 
}

```

## The Multi-threading Client/Server Test

Looking at the `main()` provided by the 42 subject to test the client, it perfectly validates the multi-threaded architecture we built.

First, there is a specific workaround used to send strings, pushing the size first, then the characters:

```cpp
std::string str = "Hello";
message2 << str.size();
for (char c : str) {
    message2 << c;
}

```

Then, we have the main test loop:

```cpp
while (!quit)
{
    client.update();

    threadSafeCout << "Client updated." << std::endl;
    std::string input;
    std::getline(std::cin, input);
}

```

Inside this loop, `client.update()` is called, followed immediately by `std::getline()`.
`std::getline` is **blocking**—it completely freezes the Main Thread until the user types something and presses Enter.

If we didn't have a background thread, the client would drop incoming messages from the server while stuck waiting for keyboard input. But because we built `_listenerThread`, our background thread happily keeps receiving messages and stuffing them into the `_inbox`. When the user finally presses Enter, the loop restarts, calls `update()`, and processes all the messages that piled up in the background.

### Iterating over Maps

As a final optimization in the networking loops, I used `auto const& pair` (or `const auto&`) for map iterations. C++ normally creates a brand-new copy of the map's key-value pair for every single iteration. By using a `const` reference, we avoid unnecessary memory allocation while promising the compiler we won't modify the data, improving overall performance.