#pragma once

#include <exception>
#include <iostream>
#include <string>

// =============================================================================
// Dummy Class to test our Pool
// =============================================================================
class Particle {
public:
  std::string name;
  float x, y, z;

  // A constructor with multiple arguments to test our variadic templates
  Particle(std::string n, float startX, float startY, float startZ)
      : name(n), x(startX), y(startY), z(startZ) {
    std::cout << "  [+] Particle '" << name << "' constructed.\n";
  }

  // Destructor to prove it gets called without freeing the raw memory
  ~Particle() { std::cout << "  [-] Particle '" << name << "' destroyed.\n"; }

  void printPosition() {
    std::cout << "      " << name << " is at (" << x << ", " << y << ", " << z
              << ")\n";
  }
};

// =============================================================================
// Dummy Class to test our Memento
// =============================================================================

class Player : public Memento {
private:
  // 1. Granting Memento the VIP pass
  friend class Memento;

  // 2. The private state data
  int _health;
  float _x;
  float _y;

  // 3. Implementing the virtual contract
  void _saveToSnapshot(Memento::Snapshot &snapshot) const override {
    snapshot << _health << _x << _y;
  }

  void _loadFromSnapshot(Memento::Snapshot &snapshot) override {
    snapshot >> _health >> _x >> _y;
  }

public:
  // Constructor
  Player(int hp, float startX, float startY)
      : _health(hp), _x(startX), _y(startY) {}

  // Methods to alter the state
  void takeDamage(int amount) {
    _health -= amount;
    if (_health < 0)
      _health = 0;
  }

  void move(float dx, float dy) {
    _x += dx;
    _y += dy;
  }

  // Helper to see what is happening
  void printStatus() const {
    std::cout << "  -> Health: " << _health << " | Position: (" << _x << ", "
              << _y << ")\n";
  }
};

// =========================================================================
// 1. The Custom Event Struct
// =========================================================================
struct PlayerEvent {
  int eventType; // used like enum
  std::string playerName;

  // needed by std::map
  bool operator<(const PlayerEvent &other) const {
    if (eventType != other.eventType) {
      return eventType < other.eventType;
    }
    return playerName < other.playerName;
  }
};

// =========================================================================
// The Class we want to turn into a Singleton
// =========================================================================
class GameManager {
private:
  friend class Singleton<GameManager>;
  int _currentLevel;
  std::string _difficulty;
  // private constructor
  GameManager(int startLevel, std::string difficulty)
      : _currentLevel(startLevel), _difficulty(difficulty) {
    std::cout << "[GameManager] Initialized at Level " << _currentLevel
              << " on " << _difficulty << " difficulty.\n";
  }

public:
  void play() {
    std::cout << "--> Playing game at level " << _currentLevel << "...\n";
  }
};

// =========================================================================
// For the state machine
// =========================================================================
enum EnemyState { IDLE, CHASE, ATTACK };

// Helper function to print enum names nicely
std::string stateToString(EnemyState state) {
  if (state == IDLE)
    return "IDLE";
  if (state == CHASE)
    return "CHASE";
  if (state == ATTACK)
    return "ATTACK";
  return "UNKNOWN";
}

// =========================================================================
// The Worker Function
// =========================================================================
void workerTask(int threadID, std::string taskName) {
  // 1. Set the prefix.
  // Because threadSafeCout is 'thread_local', Thread 1's prefix
  // will NOT overwrite Thread 2's prefix! They each have their own copy.
  threadSafeCout.setPrefix("[Worker " + std::to_string(threadID) + " | " +
                           taskName + "] ");

  for (int i = 1; i <= 3; ++i) {
    // 2. Chaining multiple << operators.
    // This all goes into the thread's private invisible buffer first.
    threadSafeCout << "Processing step " << i << "..." << std::endl;

    // Sleep for a few milliseconds to force the OS to switch threads.
    // This guarantees that if our code wasn't thread-safe, the lines would
    // overlap!
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
}

// =========================================================================
// Thread
// =========================================================================
void myFunction1() {
  for (int i = 0; i < 5; ++i) {
    threadSafeCout << "Hello from Function1, iteration " << i << std::endl;
  }
}

void myFunction2() {
  for (int i = 0; i < 5; ++i) {
    threadSafeCout << "Hello from Function2, iteration " << i << std::endl;
  }
}

void pool_test() {
  std::cout << "\n\n================================\n";
  std::cout << "============ POOL ==============\n";
  std::cout << "=== 1. Initializing Pool ===\n";
  // We use our parameterized constructor to allocate space for 3 Particles
  Pool<Particle> particlePool(3);
  std::cout << "Pool created with capacity for 3 particles.\n\n";

  std::cout << "=== 2. Acquiring Objects ===\n";
  {
    // We use a scope block { } to test the automatic destruction

    // This perfectly forwards the string and 3 floats to the Particle
    // constructor
    auto p1 = particlePool.acquire("Alpha", 1.0f, 2.0f, 3.0f);
    auto p2 = particlePool.acquire("Beta", 0.0f, 0.0f, 0.0f);

    // Test the overloaded -> operator
    if (p1) {
      p1->printPosition();
    }
    if (p2) {
      p2->printPosition();
    }

    std::cout << "\n=== 3. Objects going out of scope ===\n";
    // When this block ends, p1 and p2 are destroyed.
    // Their destructors should run, releasing the slots back to the pool.
  }

  std::cout << "\n=== 4. Acquiring Again (Reusing Memory) ===\n";
  // This should instantly slot into the memory that 'Alpha' or 'Beta' just
  // vacated!
  auto p3 = particlePool.acquire("Gamma", 9.9f, 9.9f, 9.9f);
  if (p3) {
    p3->printPosition();
  }
  std::cout << "\n=== 5. End of Program ===\n";
  // p3 will be automatically destroyed as main() exits.

  WorkerPool pool(4);

  auto job = []() {
    threadSafeCout << "Executing job on thread: " << std::this_thread::get_id()
                   << std::endl;
  };

  for (int i = 0; i < 1000; ++i) {
    pool.addJob(job);
  }

  std::this_thread::sleep_for(
      std::chrono::seconds(2)); // Wait for jobs to finish
}

class TestObject {
public:
  int x;
  std::string y;

  friend DataBuffer &operator<<(DataBuffer &p_buffer,
                                const TestObject &p_object) {
    p_buffer << p_object.x << p_object.y;
    return p_buffer;
  }

  friend DataBuffer &operator>>(DataBuffer &p_buffer, TestObject &p_object) {
    p_buffer >> p_object.x >> p_object.y;
    return p_buffer;
  }
};

void databuffer_test() {
  DataBuffer myBuffer;

  TestObject obj1;
  obj1.x = 42;
  obj1.y = "Hello";

  TestObject obj2;
  obj2.x = 99;
  obj2.y = "World";

  myBuffer << obj1 << obj2;

  TestObject deserializedObj1, deserializedObj2, deserializedObj3;

  // This should work as expected
  try {
    myBuffer >> deserializedObj1 >> deserializedObj2;
    std::cout << "Deserialized obj1: x = " << deserializedObj1.x
              << ", y = " << deserializedObj1.y << std::endl;
    std::cout << "Deserialized obj2: x = " << deserializedObj2.x
              << ", y = " << deserializedObj2.y << std::endl;
  } catch (const std::exception &e) {
    std::cout << "Caught exception: " << e.what() << std::endl;
  }

  // This should throw an exception because there are no more objects to
  // deserialize
  try {
    myBuffer >> deserializedObj3;
    std::cout << "Deserialized obj3: x = " << deserializedObj3.x
              << ", y = " << deserializedObj3.y << std::endl;
  } catch (const std::exception &e) {
    std::cout << "Caught exception: " << e.what()
              << std::endl; // This line should be executed
  }
  std::cout << "\n\n================================\n";
  std::cout << "============ DATABUFFER ===========\n";
  std::cout << "=== 1. Creating DataBuffer ===\n";
  DataBuffer buffer;

  // The data we want to save/send
  int playerHealth = 100;
  float playerX = 45.5f;
  bool isPoisoned = true;

  std::cout << "Writing to buffer:\n";
  std::cout << "  Health : " << playerHealth << "\n";
  std::cout << "  X Pos  : " << playerX << "\n";
  std::cout << "  Poison : " << (isPoisoned ? "true" : "false") << "\n\n";

  std::cout << "=== 2. Serializing (Writing) ===\n";
  // Chain the writes together!
  buffer << playerHealth << playerX << isPoisoned;

  // Let's see how big the cassette tape is:
  // int (4) + float (4) + bool (1) = 9 bytes
  std::cout << "Buffer size is now: " << buffer.size() << " bytes.\n\n";

  std::cout << "=== 3. Deserializing (Reading) ===\n";
  // Completely empty variables to prove we are reading from the buffer
  int outHealth = 0;
  float outX = 0.0f;
  bool outPoisoned = false;

  // We MUST read in the exact same order: int -> float -> bool
  buffer >> outHealth >> outX >> outPoisoned;

  std::cout << "Read from buffer:\n";
  std::cout << "  Health : " << outHealth << "\n";
  std::cout << "  X Pos  : " << outX << "\n";
  std::cout << "  Poison : " << (outPoisoned ? "true" : "false") << "\n\n";

  std::cout << "=== 4. Safety Check ===\n";
  int extraData;
  std::cout << "Attempting to read past the end of the buffer...\n";
  // This should safely fail and print your warning!
  buffer >> extraData;
}

class TestClass : public Memento {
  friend class Memento;

public:
  int x;
  std::string y;

private:
  void _saveToSnapshot(Snapshot &snapshotToFill) const override {
    snapshotToFill << x << y;
  }

  void _loadFromSnapshot(Snapshot &snapshot) override { snapshot >> x >> y; }
};

void memento_test() {
  TestClass myObject;
  myObject.x = 42;
  myObject.y = "Hello";

  // Save the current state
  TestClass::Snapshot savedState = myObject.save();

  // Modify the object
  myObject.x = 100;
  myObject.y = "World";

  // Output the modified object
  // Expected Output: "Current state: x = 100, y = World"
  std::cout << "Current state: x = " << myObject.x << ", y = " << myObject.y
            << std::endl;

  // Restore the object to its saved state
  myObject.load(savedState);

  // Output the restored object
  // Expected Output: "Restored state: x = 42, y = Hello"
  std::cout << "Restored state: x = " << myObject.x << ", y = " << myObject.y
            << std::endl;
  // --------------------- Memento ---------------------
  std::cout << "\n\n================================\n";
  std::cout << "============ Memento ===========\n";
  std::cout << "=== 1. Starting New Game ===\n";
  Player myPlayer(100, 10.0f, 20.0f);
  myPlayer.printStatus();

  std::cout << "\n=== 2. Hitting a Checkpoint (Saving) ===\n";
  // We call the public save() method inherited from Memento
  Memento::Snapshot saveSlot1 = myPlayer.save();

  // int (4) + float (4) + float (4) = 12 bytes!
  std::cout << "Game saved successfully! Snapshot size: " << saveSlot1.size()
            << " bytes.\n";

  std::cout << "\n=== 3. Disaster Strikes! ===\n";
  std::cout << "Player falls into a trap and takes 80 damage...\n";
  myPlayer.takeDamage(80);
  myPlayer.move(5.5f, -15.0f);
  myPlayer.printStatus();

  std::cout << "\n=== 4. Reloading Checkpoint ===\n";
  std::cout << "Loading saveSlot1...\n";
  // We call the public load() method, which safely reads our copy
  myPlayer.load(saveSlot1);
  myPlayer.printStatus();

  std::cout << "\n=== 5. Verifying Snapshot Integrity ===\n";
  std::cout << "Did the snapshot survive the load? Let's load it AGAIN!\n";
  myPlayer.takeDamage(99);  // Mess up the state again
  myPlayer.load(saveSlot1); // Load the exact same snapshot a second time
  myPlayer.printStatus();
  std::cout << "Success! The snapshot is perfectly intact.\n";
}

enum class EventType { EVENT_ONE, EVENT_TWO, EVENT_THREE };

void observer_test() {
  Observer<EventType> observer;

  // Subscribe to EVENT_ONE
  observer.subscribe(EventType::EVENT_ONE,
                     []() { std::cout << "Event One triggered" << std::endl; });

  // Subscribe first lambda to EVENT_TWO
  observer.subscribe(EventType::EVENT_TWO, []() {
    std::cout << "Event Two triggered (First subscriber)" << std::endl;
  });

  // Subscribe second lambda to EVENT_TWO
  observer.subscribe(EventType::EVENT_TWO, []() {
    std::cout << "Event Two triggered (Second subscriber)" << std::endl;
  });

  // Triggering EVENT_ONE
  std::cout << "Notify EVENT_ONE" << std::endl;
  observer.notify(EventType::EVENT_ONE); // Output: "Event One triggered"

  // Triggering EVENT_TWO
  std::cout << "Notify EVENT_TWO" << std::endl;
  observer.notify(EventType::EVENT_TWO);
  // Output:
  // "Event Two triggered (First subscriber)"
  // "Event Two triggered (Second subscriber)"
  // The order may differ

  // Triggering EVENT_THREE (No subscriber)
  std::cout << "Notify EVENT_THREE" << std::endl;
  observer.notify(
      EventType::EVENT_THREE); // Output: None, as there are no subscribers
                               // --------------------- Observer
                               // ---------------------
  std::cout << "\n\n================================\n";
  std::cout << "============ Observer ===========\n";
  std::cout << "=== 1. Starting ===\n";
  // Our central event manager, templated to use our custom struct
  Observer<PlayerEvent> gameEvents;

  // Define some specific event signatures
  PlayerEvent aliceLevelUp = {1, "Alice"};
  PlayerEvent bobLevelUp = {1, "Bob"};
  PlayerEvent aliceDeath = {2, "Alice"};

  std::cout << "=== PHASE 1: Subscribing to Events ===\n";

  // UI System subscribes to Alice's level up
  gameEvents.subscribe(aliceLevelUp, []() {
    std::cout << "[UI System] FLASHING CONGRATS FOR ALICE!\n";
  });

  // Audio System ALSO subscribes to Alice's level up
  gameEvents.subscribe(aliceLevelUp, []() {
    std::cout << "[Audio System] Playing level-up chime for Alice!\n";
  });

  // UI System subscribes to Bob's level up
  gameEvents.subscribe(bobLevelUp, []() {
    std::cout << "[UI System] Flashing congrats for Bob!\n";
  });

  std::cout << "Subscribers registered successfully.\n\n";

  std::cout << "=== PHASE 2: Triggering Events ===\n";

  std::cout << "--> Action: Alice levels up!\n";
  gameEvents.notify(aliceLevelUp);
  // Expectation: Triggers both the UI and Audio lambdas for Alice.

  std::cout << "\n--> Action: Bob levels up!\n";
  gameEvents.notify(bobLevelUp);
  // Expectation: Triggers ONLY the UI lambda for Bob. Alice's audio shouldn't
  // play.

  std::cout << "\n--> Action: Alice dies!\n";
  gameEvents.notify(aliceDeath);
  // Expectation: Nothing happens! Nobody subscribed to this event,
  // and our find() method safely ignores it without crashing.

  std::cout << "\nAll events processed successfully.\n";
}

class MyClass {
public:
  MyClass(int value) {
    std::cout << "MyClass constructor, with value [" << value << "]"
              << std::endl;
  }

  void printMessage() { std::cout << "Hello from MyClass" << std::endl; }
};

void singleton_test() {

  std::cout << "\n\n================================\n";
  std::cout << "============ SINGLETON ===========\n";
  std::cout << "=== 1. Initializing the Singleton ===\n";

  // We pass the arguments (int, std::string) perfectly to the private
  // constructor using the variadic template!
  Singleton<GameManager>::instantiate(1, "Hardcore");

  std::cout << "\n=== 2. Accessing the Instance ===\n";

  // We grab the global pointer to our one and only GameManager
  GameManager *myGame = Singleton<GameManager>::instance();
  if (myGame != nullptr) {
    myGame->play();
  }

  std::cout << "\n=== 3. Trying to break the Singleton Rule ===\n";

  try {
    std::cout << "Attempting to instantiate a second GameManager...\n";
    // This should trigger our exception!
    Singleton<GameManager>::instantiate(5, "Easy");
  } catch (const std::exception &e) {
    std::cerr << "EXCEPTION CAUGHT: " << e.what() << "\n";
  }

  // (Optional) Standard compiler check:
  // Uncommenting the line below will cause a COMPILER ERROR because the
  // constructor is private!
  // GameManager illegalManager(10, "Normal");

  try {
    // This should throw an exception as instance is not yet created
    Singleton<MyClass>::instance();
  } catch (const std::exception &e) {
    std::cout << "Exception: " << e.what()
              << std::endl; // Output: "Exception: Instance not yet created"
  }

  Singleton<MyClass>::instantiate(42); // Setting up the instance

  Singleton<MyClass>::instance()
      ->printMessage(); // Output: "Hello from MyClass"

  try {
    // This should throw an exception as instance is already created
    Singleton<MyClass>::instantiate(100);
  } catch (const std::exception &e) {
    std::cout << "Exception: " << e.what()
              << std::endl; // Output: "Exception: Instance already created"
  }
}

enum class State { Idle, Running, Paused, Stopped };

void state_machine_test() {
  StateMachine<State> sm;

  sm.addState(State::Idle);
  sm.addState(State::Running);
  sm.addState(State::Paused);
  sm.addState(State::Stopped);

  sm.addAction(State::Idle,
               [] { std::cout << "System is idle." << std::endl; });
  sm.addAction(State::Running,
               [] { std::cout << "System is running." << std::endl; });
  sm.addAction(State::Paused,
               [] { std::cout << "System is paused." << std::endl; });
  // No addAction for State::Stopped, it will use the default empty lambda

  sm.addTransition(State::Idle, State::Running, [] {
    std::cout << "Transitioning from Idle to Running." << std::endl;
  });
  sm.addTransition(State::Running, State::Paused, [] {
    std::cout << "Transitioning from Running to Paused." << std::endl;
  });
  sm.addTransition(State::Paused, State::Running, [] {
    std::cout << "Transitioning from Paused to Running." << std::endl;
  });
  // No addTransition for State::Stopped

  sm.update(); // Should print: "System is idle."
  sm.transitionTo(
      State::Running); // Should print: "Transitioning from Idle to Running."
  sm.update();         // Should print: "System is running."
  sm.transitionTo(
      State::Paused); // Should print: "Transitioning from Running to Paused."
  sm.update();        // Should print: "System is paused."

  // Transitioning to and from the new State::Stopped
  try {
    sm.transitionTo(State::Stopped); // Should not print any transition message,
                                     // and throw an exception
  } catch (const std::invalid_argument &e) {
    std::cout << "Exception caught: " << e.what()
              << std::endl; // Handle state not found
  }

  try {
    sm.transitionTo(State::Stopped); // Should not print anything, default empty
                                     // lambda is executed
  } catch (const std::invalid_argument &e) {
    std::cout << "Exception caught: " << e.what()
              << std::endl; // Handle state not found
  }

  try {
    sm.transitionTo(State::Running); // Should not print any transition message,
                                     // and throw an exception
  } catch (const std::invalid_argument &e) {
    std::cout << "Exception caught: " << e.what()
              << std::endl; // Handle state not found
  }
  // --------------------- State Machine ---------------------
  std::cout << "\n\n================================\n";
  std::cout << "============ State Machine ===========\n";
  std::cout << "=== 1. Starting ===\n";
  StateMachine<EnemyState> ai;

  std::cout << "=== PHASE 1: Setting up the Machine ===\n";

  // Add valid states (The first one added, IDLE, becomes the starting state)
  ai.addState(IDLE);
  ai.addState(CHASE);
  ai.addState(ATTACK);

  // Register Actions (What happens DURING a state)
  ai.addAction(IDLE, []() {
    std::cout << "[Action] Enemy is standing still, picking its nose...\n";
  });
  ai.addAction(CHASE, []() {
    std::cout << "[Action] Enemy is sprinting towards the player!\n";
  });
  // NOTICE: We intentionally forget to add an action for ATTACK to test our
  // exception later!

  // Register Transitions (What happens BETWEEN states)
  ai.addTransition(IDLE, CHASE, []() {
    std::cout << "[Transition] Enemy spots you! *ROARS*\n";
  });
  ai.addTransition(CHASE, ATTACK, []() {
    std::cout << "[Transition] Enemy gets close enough and draws its sword!\n";
  });

  std::cout << "\n=== PHASE 2: Running the Machine (Happy Path) ===\n";

  // We are currently in IDLE
  ai.update();

  // Move to CHASE
  std::cout << "\n--> Transitioning to CHASE...\n";
  ai.transitionTo(CHASE); // Triggers the roar
  ai.update();            // Triggers the sprinting action

  // Move to ATTACK
  std::cout << "\n--> Transitioning to ATTACK...\n";
  ai.transitionTo(ATTACK); // Triggers drawing the sword

  std::cout << "\n=== PHASE 3: Testing the Exceptions (Error Path) ===\n";

  std::cout << "\n--> Test A: Missing Update Action\n";
  try {
    // We are in ATTACK, but we never registered an addAction for ATTACK!
    ai.update();
  } catch (const std::exception &e) {
    std::cerr << "EXCEPTION CAUGHT: " << e.what() << "\n";
  }

  std::cout << "\n--> Test B: Invalid Transition\n";
  try {
    // We are in ATTACK. We never registered a transition from ATTACK back to
    // IDLE!
    std::cout << "Trying to force transition from ATTACK to IDLE...\n";
    ai.transitionTo(IDLE);
  } catch (const std::exception &e) {
    std::cerr << "EXCEPTION CAUGHT: " << e.what() << "\n";
  }
}
