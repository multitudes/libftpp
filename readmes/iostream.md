# IOStream

In C++, **iostream** simply stands for **Input/Output Stream**.

For this project, I needed to ensure that multiple threads wouldn't scramble their output when printing to the console simultaneously.

## Thread Safe IOStream

To solve this, I built a wrapper class that looks and acts exactly like `std::cout`. It intercepts the data, organizes it, adds a thread-specific prefix, and safely hands it over to the real `std::cout` all at once.

Here is a breakdown of how the architecture works:

### The Buffer (`std::ostringstream _buffer`)

Because multiple threads are constantly competing to print, each thread gets its own `ostringstream` (Output String Stream). This acts as a private waiting room for the text before it actually hits the console.

### The Standard Overload (`operator<<(const T& value)`)

When a thread types `threadSafeCout << "Hello"`, this function grabs "Hello" and shoves it into the thread's private `_buffer`. It does **not** print to the screen yet, and it does **not** lock the console. It merely gathers the data.

### The Trigger Overload (`operator<<(std::ostream& (*manip)(std::ostream&))`)

This overload has a very specific job: catching **Manipulators**. The most famous of these is `std::endl`, which isn't a string, but actually a function that means "End the line and flush the stream."
When our class detects `std::endl`, the real work happens: it grabs the global lock, prints the thread's prefix, dumps the entire contents of the `_buffer` to `std::cout` in a single uninterrupted block, applies the `std::endl`, and then unlocks.

### The `thread_local` Keyword

By instantiating `threadSafeCout` with the `thread_local` keyword, the compiler secretly creates a brand-new, private copy of the object for every single new thread that is spawned. This guarantees that their buffers never mix.

### The `static std::mutex`

Mutex stands for Mutual Exclusion. Because I made the mutex `static`, it means that even though `thread_local` might create 100 different copies of our `ThreadSafeIOStream` object, they all share this exact same, single global lock.

### Safe Unlocking (`std::lock_guard`)

In C (using `pthreads`), if you call `pthread_mutex_lock()` and forget to `pthread_mutex_unlock()`, the program will freeze forever in a deadlock. In this C++ implementation, there are no manual `unlock()` calls.

I used `std::lock_guard`. Because of RAII (Resource Acquisition Is Initialization), the moment the `lock_guard` goes out of scope at the end of the function, its destructor automatically unlocks the mutex.

### Other Specialized Mutexes

While researching this, I also looked into the other specialized C++ mutexes that work with `lock_guard` for different scenarios:

* **`std::recursive_mutex`:** Normally, if a thread locks a mutex and then accidentally tries to lock it *again* (like in a recursive function), the program deadlocks. A recursive mutex allows the same thread to lock it multiple times safely.
* **`std::timed_mutex`:** A mutex that allows you to specify a timeout. If it can't acquire the lock within a set limit (e.g., 5 seconds), it gives up instead of waiting forever.
* **`std::shared_mutex` (C++17):** Used for "Read/Write" locks. It allows multiple threads to read data simultaneously, but enforces exclusive access when a single thread needs to write.
