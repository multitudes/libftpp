# Threads

## ThreadSafeQueue

I implemented a wrapper around the standard `std::queue` (or `std::deque`) to safely push and pop elements from the front and back. To ensure thread safety, every method that accesses or modifies the underlying container is protected by a mutex, typically using `std::lock_guard<std::mutex> lock(_mutex);` to automatically handle locking and unlocking within the scope.

## Thread

For this module, I built a custom wrapper around C++'s standard `std::thread`. While wrapping an existing standard library feature might seem redundant, the subject requires a few specific architectural twists that make this necessary.

### 1. The Delayed Launch

Normally, a `std::thread` starts running the exact moment it is created. However, the subject requires that our constructor only sets up the thread data and waits for an explicit `start()` call to actually launch the execution.

### 2. The Integration Hook (Thread-Local Prefix)

The custom thread name needs to be passed to our `ThreadSafeIOStream` as a prefix. Because `threadSafeCout` uses the `thread_local` keyword, every thread gets its own private instance of it in its personal stack memory (even though the heap is shared across all threads).

This creates a catch: we cannot set the prefix from the main thread before the worker starts. If we call `threadSafeCout.setPrefix()` inside the constructor, we are just modifying the *main* thread's prefix. The prefix must be set from *inside* the new thread, right as it boots up, but before it runs the user's function.

To solve this, I used a lambda function inside the `start()` method to act as a middleman:

```cpp
void Thread::start() {
    // We launch a new thread, but instead of the user's function, 
    // we give it a custom lambda that captures our class variables!
    _thread = std::thread( [this]() {
        
        // 1. We are now INSIDE the new thread! Set the prefix.
        threadSafeCout.setPrefix("[" + this->_name + "] ");
        
        // 2. Execute the function the user gave us in the constructor.
        this->_functToExecute();
        
    });
}

```

### 3. Safe Shutdown

Finally, the `stop()` method acts as a safety net. If a `std::thread` is destroyed while it is still running without being properly joined, the program will crash with a `std::terminate` error. Calling `stop()` pauses the main program and waits patiently (`join()`) for the specific thread to finish its task before cleaning it up.

## Workers Pool

> Manages worker threads to execute jobs

Spawning threads is an expensive OS-level operation. Asking the operating system to allocate memory, set up stack space, and create a system-level thread context every time a background task arrives is incredibly slow. It’s like hiring a new employee for a 5-minute job and immediately firing them.

Instead, I implemented a Worker Pool. We "hire" a fixed team of workers (usually matching the CPU core count) when the program starts. They sit idle in the background. When a task arrives, a worker wakes up, executes it, and goes right back to sleep.

### Why `std::vector<std::unique_ptr<Thread>>`?

To store these workers, I couldn't just use a standard `std::vector<Thread>`.
C++ intentionally deletes the copy constructor for `std::thread` because you cannot physically copy a running OS thread. Whenever a `std::vector` runs out of capacity, it allocates a larger chunk of memory and copies its elements over. If it holds raw threads, this breaks.

Here is why the architecture specifically uses a vector of unique pointers:

* **`std::vector`**: To hold the pool of workers.
* **`Pointers`**: To bypass the vector's copying mechanism, since copying a thread is illegal.
* **`std::unique_ptr`**: To guarantee strict, exclusive ownership. If we used raw pointers (`Thread*`), we would have to manually loop through and `delete` every thread in the destructor, risking severe memory leaks if the program crashed early. The smart pointer cleans up after itself automatically when the vector is destroyed. (I also specifically avoided `std::shared_ptr` here, as shared ownership implies multiple classes need simultaneous ownership of the same thread, which is not the case).

### The Lambda Bridge for `IJobs`

The subject required an `IJobs` interface—a classic Object-Oriented approach. However, I wanted my underlying queue architecture to be modern C++14, relying on `std::function`. To satisfy both the subject's requirements and modern design principles, I used a capturing lambda as a bridge.

The lambda captures a smart pointer to the `IJobs` object and calls `.execute()` directly on the worker thread:

```cpp
auto mySubjectJob = std::make_shared<HeavyCalculationJob>();
pool.addJob([mySubjectJob]() { mySubjectJob->execute(); });

```

## PersistentWorker

> A thread that continuously performs a set of tasks defined by the user.

For standard operations, `std::lock_guard` is perfect because it automatically unlocks the mutex when it goes out of scope. However, the `PersistentWorker` requires a loop that continuously polls and executes tasks, which means we need granular control over exactly when the lock is held and released.

To achieve this, I switched to `std::unique_lock`:
`std::unique_lock<std::mutex> lock(this->_mutex);`
This provides the flexibility to manually call `lock.unlock()` mid-scope, ensuring we don't accidentally freeze the rest of the program while the persistent worker is executing a long-running task.
