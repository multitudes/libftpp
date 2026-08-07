#pragma once

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

#include <exception>
#include <iostream>
#include <string>

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
