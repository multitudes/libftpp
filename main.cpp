#include "libftpp.hpp"
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

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

  // --------------------- iostream ---------------------
  std::cout << "\n\n================================\n";
  std::cout << "============ thread safe iostream ===========\n";
  std::cout << "=== 1. Starting ===\n";
  std::cout << "=== PHASE 1: Launching Threads ===\n\n";

  // Create a vector to hold our threads
  std::vector<std::thread> workers;

  // Spawn 3 threads, passing them an ID and a mock task name
  workers.push_back(std::thread(workerTask, 1, "Audio"));
  workers.push_back(std::thread(workerTask, 2, "Physics"));
  workers.push_back(std::thread(workerTask, 3, "Network"));

  // Wait for all threads to finish their work
  for (auto &worker : workers) {
    if (worker.joinable()) {
      worker.join();
    }
  }

  std::cout << "\n=== PHASE 2: Testing Input (Prompt) ===\n\n";

  // We can also test the prompt feature from the main thread!
  threadSafeCout.setPrefix("[Main Thread] ");
  int userAge = 0;

  // Uncomment this line below if you want to test the interactive input!
  threadSafeCout.prompt("Enter your age to exit: ", userAge);

  threadSafeCout << "Test complete. User age entered: " << userAge << std::endl;

  // --------------------- thread safe queue ---------------------
  std::cout << "\n\n================================\n";
  std::cout << "============ thread safe queue ===========\n";
  std::cout << "=== 1. Starting ===\n";

  ThreadSafeQueue<std::string> jobQueue;

  std::cout << "=== 1. Testing Pushes ===\n";
  jobQueue.push_back("Task B (Back)");
  jobQueue.push_front("Task A (Front)");
  jobQueue.push_back("Task C (Back)");

  std::cout << "Tasks added successfully.\n\n";

  std::cout << "=== 2. Testing Pops ===\n";
  // Expected order based on our pushes: A, B, C
  std::cout << "Popped: " << jobQueue.pop_front() << "\n"; // Should be A
  std::cout << "Popped: " << jobQueue.pop_front() << "\n"; // Should be B
  std::cout << "Popped: " << jobQueue.pop_front() << "\n"; // Should be C

  std::cout << "\n=== 3. Testing Exception ===\n";
  try {
    std::cout << "Attempting to pop from an empty queue...\n";
    jobQueue.pop_front(); // This should trigger the throw!
  } catch (const std::exception &e) {
    std::cerr << "EXCEPTION CAUGHT: " << e.what() << "\n";
  }

  // --------------------- Thread ---------------------
  std::cout << "\n\n================================\n";
  std::cout << "============ Thread ===========\n";
  std::cout << "=== 1. Starting ===\n\n";
  Thread thread1("Thread1", myFunction1);
  Thread thread2("Thread2", myFunction2);

  thread1.start();
  thread2.start();

  thread1.stop();
  thread2.stop();

  // --------------------- WorkerPool ---------------------
  std::cout << "\n\n================================\n";
  std::cout << "============ WorkerPool ===========\n";
  std::cout << "=== 1. Starting ===\n\n";
  WorkerPool pool(4);

  auto job = []() {
    threadSafeCout << "Executing job on thread: " << std::this_thread::get_id()
                   << std::endl;
    // Simulate a "heavy" computation by sleeping for 5 milliseconds
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  };

  for (int i = 0; i < 1000; ++i) {
    pool.addJob(job);
  }

  std::this_thread::sleep_for(
      std::chrono::seconds(2)); // Wait for jobs to finish

  // --------------------- PersistentWorker ---------------------
  std::cout << "\n\n================================\n";
  std::cout << "============ PersistentWorker ===========\n";
  std::cout << "=== 1. Starting ===\n\n";

  PersistentWorker worker;

  auto task1 = []() { threadSafeCout << "Executing Task 1" << std::endl; };

  auto task2 = []() { threadSafeCout << "Executing Task 2" << std::endl; };

  worker.addTask("Task1", task1);
  worker.addTask("Task2", task2);

  std::this_thread::sleep_for(std::chrono::seconds(1));

  worker.removeTask("Task1");

  std::this_thread::sleep_for(std::chrono::seconds(1));

  std::cout << "\n\n================================\n";
  std::cout << "============ ENDING ===========\n";
  return 0;
}