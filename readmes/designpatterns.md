# Design Patterns

> Design Patterns: Elements of Reusable Object-Oriented Software (1994) is a software engineering book describing software design patterns. The book was written by Erich Gamma, Richard Helm, Ralph Johnson, and John Vlissides, with a foreword by Grady Booch. The book is divided into two parts, with the first two chapters exploring the capabilities and pitfalls of object-oriented programming, and the remaining chapters describing 23 classic software design patterns. The book includes examples in C++ and Smalltalk. - wiki

## Memento

The Memento pattern allows us to take snapshots of an object's internal state. The `Memento` class offers public `save()` and `load()` methods, using our previously implemented `DataBuffer` as the `Snapshot` under the hood.

```cpp
using Snapshot = DataBuffer;

```

### The Non-Virtual Interface (NVI) Idiom

In C++, there is no official `interface` keyword. By convention, a class is a "pure interface" only if every function is pure virtual (`= 0`) and it contains no implemented code. Because our `Memento` class has concrete `save()` and `load()` methods, it crosses into being an abstract class.

This leads us to a C++ design pattern called the **Non-Virtual Interface (NVI)** (a specific version of the Template Method Pattern). The base `Memento` class maintains control over the flow. When someone calls `save()`, the base class can do extra setup (like logging time or locking a mutex). Only then does it call the private `_saveToSnapshot()` to let the child class write its specific variables into the buffer.

```cpp
#pragma once

#include "data_buffer.hpp"

class Memento {
public:
    using Snapshot = DataBuffer;

    // Virtual destructor is mandatory for classes meant to be inherited!
    virtual ~Memento() = default;

    // The public interface for the user
    Snapshot save() const; 
    void load(const Snapshot& state);

private:
    // The "Pure Virtual" methods. 
    // The '= 0' means Memento has no code for these; the child MUST provide it.
    virtual void _saveToSnapshot(Snapshot& snapshot) const = 0;
    virtual void _loadFromSnapshot(Snapshot& snapshot) = 0;
};

```

### The `friend` Keyword

To make this work, the child class must explicitly grant `Memento` access to its private methods. We do this using the `friend` keyword:

```cpp
class Player : public Memento {
private:
    // The explicit invite! Grants Memento access to the private methods below.
    friend class Memento; 

    int _health;
    float _x, _y;

    // The private implementations of the virtual contract
    void _saveToSnapshot(Memento::Snapshot& snapshot) const override {
        snapshot << _health << _x << _y;
    }

    void _loadFromSnapshot(Memento::Snapshot& snapshot) override {
        snapshot >> _health >> _x >> _y;
    }
};

```

### Mapping to the Classic Implementation

If you look at the classic UML diagram for Memento, there are three actors. Here is how they map to our code:

* **The Originator:** The object whose state needs saving. (Our `Player` or `TestClass`).
* **The Memento:** The locked box containing the saved data. (Our `Snapshot` / `DataBuffer`).
* **The Caretaker:** The manager holding onto the saves. (Our `main()` function or a save-manager holding a `std::vector<Snapshot>`).

---

## The Observer Pattern

> In software design and software engineering, the observer pattern is a software design pattern in which an object, called the subject, maintains a list of its dependents, called observers, and automatically notifies them of any state changes... - wiki

For this project, I implemented a modern version of this pattern using `std::map`, variadic templates, `std::function`, and lambdas. This is how modern engines (like Unity or Unreal) handle observers, essentially creating a **Publish/Subscribe (Pub/Sub) Event Bus** rather than the strict 1994 "Gang of Four" Observer Pattern.

### Classic vs. Modern

In the classic OOP Observer pattern, there is no middleman. Objects talk directly to each other using strict Inheritance Interfaces. The Subject holds pointers to Observers and calls `notify()`, forcing Observers to reach back into the Subject to get the state. This creates tight coupling and inheritance bloat.

Our modern approach maps to the classic ideas but removes the friction:

* **UML `Subject::attach()**` $\rightarrow$ our `subscribe(event, lambda)`
* **UML `Subject::notify()**` $\rightarrow$ our `notify(event)`
* **UML `Observer::update()**` $\rightarrow$ our `std::function` (Lambdas)

We get **Zero Coupling**: the `Player` doesn't know the `UI` exists, and neither requires base classes or virtual functions. They just talk to the central Event Bus.

```cpp
enum GameEvent {
    PLAYER_LEVEL_UP
};

Observer<GameEvent> globalObserver; 

// UI, Audio, and Network all subscribe to the SAME event independently
globalObserver.subscribe(PLAYER_LEVEL_UP, []() {
    std::cout << "[UI System] Flashing 'LEVEL UP!' on screen.\n";
});
globalObserver.subscribe(PLAYER_LEVEL_UP, []() {
    std::cout << "[Audio System] Playing fanfare.wav loudly.\n";
});
globalObserver.subscribe(PLAYER_LEVEL_UP, []() {
    std::cout << "[Network System] Saving new level to the cloud.\n";
});

// The player triggers all three without ever talking to them directly
class Player {
public:
    void levelUp() {
        globalObserver.notify(PLAYER_LEVEL_UP);
    }
};

```

### Avoiding Hidden Map Memory Leaks

When writing the `notify` method, using `_subscribers[event]` creates a hidden memory leak. In C++, map square brackets `[]` guarantee you get an item back. If the event does not exist, the map instantly inserts a new, empty `std::vector` into the dictionary, taking up RAM.

To make `notify` perfectly secure, I use `find()`, which is guaranteed not to alter the map:

```cpp
// The 'const' promises we won't change the dictionary
void notify(const TEvent& event) const { 
    auto it = _subscribers.find(event); 
    
    if (it != _subscribers.end()) {
        // Range-based for loop using const reference
        for (const auto& lambda : it->second) {
            lambda();
        }
    }
}

```

When `find()` succeeds, it returns an iterator pointing to a `std::pair`.

* `it->first` is the Key (the `TEvent`).
* `it->second` is the Value (the `std::vector` of lambdas).
By using `const auto&`, we loop through the lambdas by reference, avoiding slow copies and ensuring we don't accidentally overwrite them.

### Custom Structs as Map Keys

`std::map` uses a Red-Black Tree behind the scenes, meaning it is **ordered**. If we use a plain `struct` as an event key, the compiler will throw an error because it doesn't know how to sort it. We have to implement an `operator<`:

```cpp
struct PlayerEvent {
    int eventType;           
    std::string playerName;  

    bool operator<(const PlayerEvent& other) const {
        if (eventType != other.eventType) {
            return eventType < other.eventType;
        }
        return playerName < other.playerName;
    }
};

```

*(Note: `std::unordered_map` is possible but requires more boilerplate—specifically `operator==` and a custom Hash function).*

---

## Singleton

The Singleton pattern ensures a class has only **one instance** globally (e.g., one `AudioEngine` or one `GameManager`). I wrote a generic template (`singleton.hpp`) to turn any class into a Singleton.

* **`instantiate(TArgs&& p_args)`**: Passes arguments to the constructor. If called a second time, it throws an exception.
* **`instance()`**: Provides global access, returning the pointer to our single instance.

To enforce the pattern, the target class must make its constructor private and explicitly invite the Singleton in using the `friend` keyword: `friend class Singleton<TType>;`. Without a private constructor, any developer could accidentally instantiate a second `GameManager` manually.

---

## The Finite State Machine

Also known as the State Pattern, this is used in almost every video game character controller to alter behavior based on internal state (e.g., Idle, Chase, Attack), completely replacing messy `if/else` logic.

### The Methods

I built a templated `StateMachine` to control behavior cleanly:

* **`addState(const TState& state)`**: Registers available states.
* **`addAction(const TState& state, const std::function<void()>& lambda)`**: Maps logic to a state (e.g., pathfinding logic for `CHASE`).
* **`update()`**: Called every frame. Checks current state and executes the registered lambda. (Throws an exception if no action is registered).
* **`addTransition(const TState& startState, const TState& finalState, const std::function<void()>& lambda)`**: Registers behavior for the *in-between* moments (e.g., playing a sound exactly when transitioning `IDLE -> CHASE`).
* **`transitionTo(const TState& state)`**: Executes the transition lambda and switches the internal state. (Throws an exception if the transition wasn't set up).

### Understanding the Classic UML

If you look at the classic Gang of Four UML diagram for the State Pattern, it serves as a visual blueprint for **Delegation**:

1. **Context (Top Left Box):** This is our `Player`. The diamond line connecting `Context` to `State` means **"Has-A"** (Aggregation/Composition). The Context physically holds a pointer to a `State` object. The dog-eared note shows that `Context::request()` delegates the work by calling `state.handle()`.
2. **State (Top Right Box):** The abstract interface (often italicized in UML). It provides the blueprint (`+handle()`).
3. **ConcreteState A & B (Bottom Boxes):** Specific implementations (like `IdleState` or `JumpState`). The dashed line with the open triangle means **"Is-A"** (Inheritance). They provide the actual code for `+handle()`.