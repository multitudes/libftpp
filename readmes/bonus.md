# libftcpp - Bonuses

Here is a breakdown of the bonuses I implemented for this project and the design decisions behind them.

## 1. The PPM Image Exporter

I added a PPM image exporter to visualize the Perlin noise generator we coded. PPM is a very straightforward format—it consists of a small header followed by a sequence of pixel RGB values. It’s incredibly easy to write to and can be read natively by almost any operating system, making it the perfect lightweight choice for this visualization.

## 2. Timer

I built a polling timer using C++11's `<chrono>` library. It acts passively, relying on the programmer to check if it has timed out. This perfectly fits the subject's requirements and mirrors how timers usually work inside game loops where the screen redraws continuously.

### Initialization & Under the Hood

`std::chrono::milliseconds` is a template class, not a primitive. I used **Uniform Initialization** (curly braces) for the constructor:

```cpp
Timer(long long duration_ms) 
    : _duration{std::chrono::milliseconds{duration_ms}} {
    reset(); 
}

```

This is modern C++ best practice. Unlike parentheses, curly braces prevent "narrowing conversions," forcing the compiler to throw an error if a float is accidentally passed instead of an integer.

### Using `steady_clock`

I specifically used `std::chrono::steady_clock` instead of `system_clock`.

* `steady_clock` acts as a physical stopwatch. It is strictly monotonic and ignores real-world time. If the OS time syncs with an NTP server or daylight saving time hits, `steady_clock` won't jump backward or forward.
* `system_clock` is tied to the OS wall clock, which is great for logging dates but terrible for measuring timeouts.

### Handling Timeouts and `.count()`

Subtracting timepoints in `<chrono>` returns a `std::chrono::duration` object, which safely remembers its own units (seconds, milliseconds, etc.). To get the raw integer value out of it, you have to call `.count()`.

For the polling loop, I avoided the old C-style `std::sleep()` and used `std::this_thread::sleep_for`:

```cpp
while (!myTimer.hasTimedOut()) {
    std::cout << "." << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

```

## 3. The Command Design Pattern

I implemented the Command design pattern, but I intentionally avoided the standard Wikipedia implementation in favor of a modern C++ approach.

### The Problem with the Classic Approach

The classic generic approach (like on Wikipedia) relies on pointers to member functions:
`using Action = void (Receiver::*)();`

This creates a rigid limitation: the action must return `void` and take exactly **zero parameters**. If you want a command to take arguments (e.g., `addMoney(50)`), the classic approach breaks unless you write complex, bloated template workarounds.

### The Modern Solution: Lambdas and `std::function`

Instead of using member-function pointers, I used `std::function<void()>` combined with lambdas.

```cpp
class LambdaCommand : public Command {
private:
    std::function<void()> _action;

public:
    LambdaCommand(std::function<void()> action) : _action(action) {}

    void execute() override {
        if (_action) _action();
    }
};

```

This takes advantage of **Type Erasure**. The command queue only expects a `void` function with no arguments, but lambdas let us capture whatever objects and arguments we need *before* the command goes into the queue:

```cpp
// We can mix entirely different signatures in the same queue!
commandQueue.push_back(std::make_unique<LambdaCommand>([&myPlayer, damage]() {
    myPlayer.takeDamage(damage);
}));

commandQueue.push_back(std::make_unique<LambdaCommand>([&myAudio]() {
    myAudio.playSound("jump.wav", 0.8f);
}));

```

This is the modern industry standard. It's cleaner, eliminates `->*` syntax, and allows a single queue to blindly execute functions with completely different underlying signatures.

*(Note on Inheritance: The `LambdaCommand` inherits `Command` using `public` inheritance. If you omit the `public` keyword, C++ defaults to `private` inheritance for classes, which would instantly break polymorphism and prevent storing the commands in a base-class vector.)*

## 4. Other C++ Best Practices Applied

### Bypassing "The Most Vexing Parse"

If you try to call a default constructor with empty parentheses like this:
`IVector2<int> vec4();`
The C++ compiler reads this as a function declaration returning an `IVector2<int>`, not an object instantiation. By using uniform initialization (`IVector2<int> vec4{};`), the compiler definitively knows we are creating a variable.

### Pre-Increment (`++i`) over Post-Increment (`i++`)

Throughout the loops, I defaulted to `++i`.
Post-increment (`i++`) requires the computer to make a temporary copy of the variable, increment the original, and return the copy. While modern compilers optimize this out for basic `int`s, it causes a performance hit when iterating over complex objects or STL iterators. Using `++i` strictly increments and returns, avoiding unnecessary memory overhead.

