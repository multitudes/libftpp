#include "design_patterns/singleton.hpp"
#include "libftpp.hpp"
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

// =============================================================================
// Main Test
// =============================================================================
int main() {
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

  // --------------------- Observer ---------------------
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

  std::cout << "\n\n================================\n";
  std::cout << "============ ENDING ===========\n";
  return 0;
}