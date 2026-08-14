#include "test.hpp"
#include <cmath>

std::string stateToString(EnemyState state) {
  if (state == IDLE)
    return "IDLE";
  if (state == CHASE)
    return "CHASE";
  if (state == ATTACK)
    return "ATTACK";
  return "UNKNOWN";
}

void workerTask(int threadID, std::string taskName) {
  threadSafeCout.setPrefix("[Worker " + std::to_string(threadID) + " | " +
                           taskName + "] ");
  for (int i = 1; i <= 3; ++i) {
    threadSafeCout << "Processing step " << i << "..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
}

// Particle is a convenience class defclared in the test.hpp file
// it just has a 3D coordinate.
void pool_test() {
  std::cout << "\n\n================================\n";
  std::cout << "============ POOL ==============\n";
  std::cout << "=== Initializing Pool ===\n";

  Pool<Particle> particlePool(3);
  std::cout << "Pool created with capacity for 3 particles.\n\n";

  std::cout << "=== 2. Acquiring Objects (Filling the pool) ===\n";
  auto p1 = particlePool.acquire("Alpha", 1.0f, 2.0f, 3.0f);
  auto p2 = particlePool.acquire("Beta", 0.0f, 0.0f, 0.0f);

  {
    // We open a scope for the 3rd particle
    auto p3 = particlePool.acquire("Gamma", 9.9f, 9.9f, 9.9f);

    std::cout << "\n=== 3. Testing Pool Exhaustion ===\n";
    std::cout
        << "Pool should be full now. Attempting to acquire a 4th particle...\n";

    // This should trigger your edge-case warning and return a nullptr wrapper!
    auto p4 = particlePool.acquire("Delta", 4.4f, 4.4f, 4.4f);

    if (!p4) {
      std::cout << "  -> Success! Correctly received an empty wrapper without "
                   "crashing.\n";
    }

    std::cout << "\n=== 4. Releasing Object (p3 goes out of scope) ===\n";
  } // p3 is destroyed here, freeing up exactly one slot

  std::cout << "\n=== 5. Acquiring Again (Reusing Memory) ===\n";
  // This should safely grab the slot that p3 just vacated
  auto p5 = particlePool.acquire("Epsilon", 5.5f, 5.5f, 5.5f);
  if (p5) {
    p5->printPosition();
  }
}

void databuffer_test() {
  std::cout << "\n\n================================\n";
  std::cout << "============ DATABUFFER ===========\n";
  std::cout << "=== 1. Creating DataBuffer ===\n";
  DataBuffer myBuffer;

  TestObject obj1;
  obj1.x = 42;
  obj1.y = "Hello";

  TestObject obj2;
  obj2.x = 99;
  obj2.y = "World";

  myBuffer << obj1 << obj2;

  TestObject deserializedObj1, deserializedObj2, deserializedObj3;

  // Test Object Deserialization
  try {
    myBuffer >> deserializedObj1 >> deserializedObj2;
    std::cout << "Deserialized obj1: x = " << deserializedObj1.x
              << ", y = " << deserializedObj1.y << std::endl;
    std::cout << "Deserialized obj2: x = " << deserializedObj2.x
              << ", y = " << deserializedObj2.y << std::endl;
  } catch (const std::exception &e) {
    std::cout << "Caught exception: " << e.what() << std::endl;
  }

  // Expecting a failure here since buffer is empty
  try {
    myBuffer >> deserializedObj3;
    std::cout << "Deserialized obj3: x = " << deserializedObj3.x
              << ", y = " << deserializedObj3.y << std::endl;
  } catch (const std::exception &e) {
    std::cout << "Caught expected exception: " << e.what() << std::endl;
  }

  DataBuffer buffer;

  int playerHealth = 100;
  float playerX = 45.5f;
  bool isPoisoned = true;

  std::cout << "Writing to buffer:\n";
  std::cout << "  Health : " << playerHealth << "\n";
  std::cout << "  X Pos  : " << playerX << "\n";
  std::cout << "  Poison : " << (isPoisoned ? "true" : "false") << "\n\n";

  std::cout << "=== 2. Serializing (Writing) ===\n";
  buffer << playerHealth << playerX << isPoisoned;
  std::cout << "Buffer size is now: " << buffer.size() << " bytes.\n\n";

  std::cout << "=== 3. Deserializing (Reading) ===\n";
  int outHealth = 0;
  float outX = 0.0f;
  bool outPoisoned = false;

  buffer >> outHealth >> outX >> outPoisoned;

  std::cout << "Read from buffer:\n";
  std::cout << "  Health : " << outHealth << "\n";
  std::cout << "  X Pos  : " << outX << "\n";
  std::cout << "  Poison : " << (outPoisoned ? "true" : "false") << "\n\n";

  std::cout << "=== 4. Safety Check (Buffer End) ===\n";
  int extraData;
  std::cout << "Attempting to read past the end of the buffer...\n";
  try {
    buffer >> extraData;
  } catch (const std::exception &e) {
    std::cout << "Caught expected exception: " << e.what() << "\n";
  }

  std::cout << "\n=== 5. Empty String Edge Case ===\n";
  DataBuffer stringBuffer;
  std::string emptyString = "";
  std::string normalString = "Still Works!";

  std::cout << "Writing an empty string [\"\"] followed by normal text [\""
            << normalString << "\"]...\n";
  stringBuffer << emptyString << normalString;

  std::string outEmpty;
  std::string outNormal;

  stringBuffer >> outEmpty >> outNormal;

  std::cout << "Deserialized Strings:\n";
  std::cout << "  String 1: \"" << outEmpty << "\"\n";
  std::cout << "  String 2: \"" << outNormal << "\"\n";

  if (outEmpty.empty() && outNormal == "Still Works!") {
    std::cout << "  -> Success! Empty string safely serialized without "
                 "corrupting memory.\n";
  } else {
    std::cout << "  -> FAILURE! The empty string corrupted the buffer.\n";
  }
}

void memento_test() {
  TestClass myObject;
  myObject.x = 42;
  myObject.y = "Hello";

  TestClass::Snapshot savedState = myObject.save();

  myObject.x = 100;
  myObject.y = "World";

  std::cout << "Current state: x = " << myObject.x << ", y = " << myObject.y
            << std::endl;

  myObject.load(savedState);
  std::cout << "Restored state: x = " << myObject.x << ", y = " << myObject.y
            << std::endl;

  std::cout << "\n\n================================\n";
  std::cout << "============ Memento ===========\n";
  std::cout << "=== 1. Starting New Game ===\n";
  Player myPlayer(100, 10.0f, 20.0f);
  myPlayer.printStatus();

  std::cout << "\n=== 2. Hitting a Checkpoint (Saving) ===\n";
  Memento::Snapshot saveSlot1 = myPlayer.save();
  std::cout << "Game saved successfully! Snapshot size: " << saveSlot1.size()
            << " bytes.\n";

  std::cout << "\n=== 3. Disaster Strikes! ===\n";
  std::cout << "Player falls into a trap and takes 80 damage...\n";
  myPlayer.takeDamage(80);
  myPlayer.move(5.5f, -15.0f);
  myPlayer.printStatus();

  std::cout << "\n=== 4. Reloading Checkpoint ===\n";
  std::cout << "Loading saveSlot1...\n";
  myPlayer.load(saveSlot1);
  myPlayer.printStatus();

  std::cout << "\n=== 5. Verifying Snapshot Integrity ===\n";
  std::cout << "Did the snapshot survive the load? Let's load it AGAIN!\n";
  myPlayer.takeDamage(99);
  myPlayer.load(saveSlot1);
  myPlayer.printStatus();
  std::cout << "Success! The snapshot is perfectly intact.\n";
}

void observer_test() {
  Observer<EventType> observer;

  observer.subscribe(EventType::EVENT_ONE,
                     []() { std::cout << "Event One triggered" << std::endl; });
  observer.subscribe(EventType::EVENT_TWO, []() {
    std::cout << "Event Two triggered (First subscriber)" << std::endl;
  });
  observer.subscribe(EventType::EVENT_TWO, []() {
    std::cout << "Event Two triggered (Second subscriber)" << std::endl;
  });

  std::cout << "Notify EVENT_ONE" << std::endl;
  observer.notify(EventType::EVENT_ONE);

  std::cout << "Notify EVENT_TWO" << std::endl;
  observer.notify(EventType::EVENT_TWO);

  std::cout << "Notify EVENT_THREE" << std::endl;
  observer.notify(EventType::EVENT_THREE);

  std::cout << "\n\n================================\n";
  std::cout << "============ Observer ===========\n";
  std::cout << "=== 1. Starting ===\n";
  Observer<PlayerEvent> gameEvents;

  PlayerEvent aliceLevelUp = {1, "Alice"};
  PlayerEvent bobLevelUp = {1, "Bob"};
  PlayerEvent aliceDeath = {2, "Alice"};

  std::cout << "=== PHASE 1: Subscribing to Events ===\n";

  gameEvents.subscribe(aliceLevelUp, []() {
    std::cout << "[UI System] FLASHING CONGRATS FOR ALICE!\n";
  });
  gameEvents.subscribe(aliceLevelUp, []() {
    std::cout << "[Audio System] Playing level-up chime for Alice!\n";
  });
  gameEvents.subscribe(bobLevelUp, []() {
    std::cout << "[UI System] Flashing congrats for Bob!\n";
  });

  std::cout << "Subscribers registered successfully.\n\n";
  std::cout << "=== PHASE 2: Triggering Events ===\n";

  std::cout << "--> Action: Alice levels up!\n";
  gameEvents.notify(aliceLevelUp);

  std::cout << "\n--> Action: Bob levels up!\n";
  gameEvents.notify(bobLevelUp);

  std::cout << "\n--> Action: Alice dies!\n";
  gameEvents.notify(aliceDeath);

  std::cout << "\nAll events processed successfully.\n";
}

void singleton_test() {
  std::cout << "\n\n================================\n";
  std::cout << "============ SINGLETON ===========\n";
  std::cout << "=== 1. Initializing the Singleton ===\n";

  Singleton<GameManager>::instantiate(1, "Hardcore");

  std::cout << "\n=== 2. Accessing the Instance ===\n";
  GameManager *myGame = Singleton<GameManager>::instance();
  if (myGame != nullptr) {
    myGame->play();
  }

  std::cout << "\n=== 3. Trying to break the Singleton Rule ===\n";
  try {
    std::cout << "Attempting to instantiate a second GameManager...\n";
    Singleton<GameManager>::instantiate(5, "Easy");
  } catch (const std::exception &e) {
    std::cerr << "EXCEPTION CAUGHT: " << e.what() << "\n";
  }

  try {
    Singleton<MyClass>::instance();
  } catch (const std::exception &e) {
    std::cout << "Exception: " << e.what() << std::endl;
  }

  Singleton<MyClass>::instantiate(42);
  Singleton<MyClass>::instance()->printMessage();

  try {
    Singleton<MyClass>::instantiate(100);
  } catch (const std::exception &e) {
    std::cout << "Exception: " << e.what() << std::endl;
  }
  std::cout << "\n=== 4. Testing Thread Safety (Race Condition) ===\n";

  // We will spawn 10 threads that all try to instantiate a new Singleton at the
  // exact same time.
  std::vector<std::thread> threads;
  int successCount = 0;
  int exceptionCount = 0;
  std::mutex
      coutMutex; // Just to keep our console prints from garbling together

  // Dummy class just for this thread test
  class ThreadTestClass {
    friend class Singleton<ThreadTestClass>;
    ThreadTestClass(int) {} // Private constructor
  };

  for (int i = 0; i < 10; ++i) {
    threads.push_back(
        std::thread([&successCount, &exceptionCount, &coutMutex]() {
          try {
            // ALL 10 threads hit this line at roughly the same time!
            Singleton<ThreadTestClass>::instantiate(99);

            std::lock_guard<std::mutex> lock(coutMutex);
            successCount++;
            std::cout << "[Thread " << std::this_thread::get_id()
                      << "] WINNER: Successfully instantiated the Singleton!\n";
          } catch (const std::exception &) {
            std::lock_guard<std::mutex> lock(coutMutex);
            exceptionCount++;
            // We ignore the print here so it doesn't spam, but we count the
            // exception
          }
        }));
  }

  // Wait for all threads to finish fighting
  for (auto &t : threads) {
    t.join();
  }

  std::cout << "\nRace Condition Results:\n";
  std::cout << "  Successful instantiations : " << successCount
            << " (Expected: 1)\n";
  std::cout << "  Caught exceptions         : " << exceptionCount
            << " (Expected: 9)\n";

  if (successCount == 1 && exceptionCount == 9) {
    std::cout << "  -> THREAD SAFE! Only one thread managed to create it.\n";
  } else {
    std::cout << "  -> THREAD UNSAFE! Multiple instances were created, memory "
                 "leaked.\n";
  }
}

void state_machine_test() {
  std::cout << "\n\n================================\n";
  std::cout << "============ State Machine ===========\n";
  std::cout << "=== Test Starting ===\n";
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

  sm.addTransition(State::Idle, State::Running, [] {
    std::cout << "Transitioning from Idle to Running." << std::endl;
  });
  sm.addTransition(State::Running, State::Paused, [] {
    std::cout << "Transitioning from Running to Paused." << std::endl;
  });
  sm.addTransition(State::Paused, State::Running, [] {
    std::cout << "Transitioning from Paused to Running." << std::endl;
  });

  sm.update();
  sm.transitionTo(State::Running);
  sm.update();
  sm.transitionTo(State::Paused);
  sm.update();

  try {
    sm.transitionTo(State::Stopped);
  } catch (const std::invalid_argument &e) {
    std::cout << "Exception caught: " << e.what() << std::endl;
  }

  try {
    sm.transitionTo(State::Stopped);
  } catch (const std::invalid_argument &e) {
    std::cout << "Exception caught: " << e.what() << std::endl;
  }

  try {
    sm.transitionTo(State::Running);
  } catch (const std::invalid_argument &e) {
    std::cout << "Exception caught: " << e.what() << std::endl;
  }

  StateMachine<EnemyState> ai;
  std::cout << "=== PHASE 1: Setting up the Machine ===\n";
  ai.addState(IDLE);
  ai.addState(CHASE);
  ai.addState(ATTACK);

  ai.addAction(IDLE, []() {
    std::cout << "[Action] Enemy is standing still, picking its nose...\n";
  });
  ai.addAction(CHASE, []() {
    std::cout << "[Action] Enemy is sprinting towards the player!\n";
  });

  ai.addTransition(IDLE, CHASE, []() {
    std::cout << "[Transition] Enemy spots you! *ROARS*\n";
  });
  ai.addTransition(CHASE, ATTACK, []() {
    std::cout << "[Transition] Enemy gets close enough and draws its sword!\n";
  });

  std::cout << "\n=== PHASE 2: Running the Machine (Happy Path) ===\n";
  ai.update();
  std::cout << "\n--> Transitioning to CHASE...\n";
  ai.transitionTo(CHASE);
  ai.update();

  std::cout << "\n--> Transitioning to ATTACK...\n";
  ai.transitionTo(ATTACK);

  std::cout << "\n=== PHASE 3: Testing the Exceptions (Error Path) ===\n";
  std::cout << "\n--> Test A: Missing Update Action\n";
  try {
    ai.update();
  } catch (const std::exception &e) {
    std::cerr << "EXCEPTION CAUGHT: " << e.what() << "\n";
  }

  std::cout << "\n--> Test B: Invalid Transition\n";
  try {
    std::cout << "Trying to force transition from ATTACK to IDLE...\n";
    ai.transitionTo(IDLE);
  } catch (const std::exception &e) {
    std::cerr << "EXCEPTION CAUGHT: " << e.what() << "\n";
  }
}

void printNumbers(const std::string &p_prefix) {
  threadSafeCout.setPrefix(p_prefix);
  for (int i = 1; i <= 5; ++i) {
    threadSafeCout << "Number: " << i << std::endl;
  }
}

void thread_safe_iostream_test() {
  std::string prefix1 = "[Thread 1] ";
  std::string prefix2 = "[Thread 2] ";

  std::thread thread1(printNumbers, prefix1);
  std::thread thread2(printNumbers, prefix2);

  thread1.join();
  thread2.join();

  std::cout << "\n\n================================\n";
  std::cout << "============ thread safe iostream ===========\n";
  std::cout << "=== 1. Starting ===\n";
  std::cout << "=== PHASE 1: Launching Threads ===\n\n";

  std::vector<std::thread> workers;
  workers.push_back(std::thread(workerTask, 1, "Audio"));
  workers.push_back(std::thread(workerTask, 2, "Physics"));
  workers.push_back(std::thread(workerTask, 3, "Network"));

  for (auto &worker : workers) {
    if (worker.joinable()) {
      worker.join();
    }
  }

  std::cout << "\n=== PHASE 2: Testing Input (Prompt) ===\n\n";
}

void testPush(ThreadSafeQueue<int> &p_queue, int p_value) {
  p_queue.push_back(p_value);
  std::cout << "Pushed value: " << p_value << std::endl;
}

void testPop(ThreadSafeQueue<int> &p_queue) {
  try {
    int value = p_queue.pop_front();
    std::cout << "Popped value: " << value << std::endl;
  } catch (const std::runtime_error &e) {
    std::cout << e.what() << std::endl;
  }
}

void thread_safe_queue_test() {
  std::cout << "\n\n================================\n";
  std::cout << "============ thread safe queue ===========\n";
  std::cout << "===  Starting ===\n";
  ThreadSafeQueue<int> myQueue;

  std::thread thread1(testPush, std::ref(myQueue), 10);
  std::thread thread2(testPush, std::ref(myQueue), 20);
  std::thread thread3(testPop, std::ref(myQueue));
  std::thread thread4(testPop, std::ref(myQueue));
  std::thread thread5(testPop, std::ref(myQueue));

  thread1.join();
  thread2.join();
  thread3.join();
  thread4.join();
  thread5.join();

  ThreadSafeQueue<std::string> jobQueue;
  std::cout << "=== 1. Testing Pushes ===\n";
  jobQueue.push_back("Task B (Back)");
  jobQueue.push_front("Task A (Front)");
  jobQueue.push_back("Task C (Back)");

  std::cout << "Tasks added successfully.\n\n";
  std::cout << "=== 2. Testing Pops ===\n";
  std::cout << "Popped: " << jobQueue.pop_front() << "\n";
  std::cout << "Popped: " << jobQueue.pop_front() << "\n";
  std::cout << "Popped: " << jobQueue.pop_front() << "\n";

  std::cout << "\n=== 3. Testing Exception ===\n";
  try {
    std::cout << "Attempting to pop from an empty queue...\n";
    jobQueue.pop_front();
  } catch (const std::exception &e) {
    std::cerr << "EXCEPTION CAUGHT: " << e.what() << "\n";
  }
}

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

void thread_test() {
  std::cout << "\n\n================================\n";
  std::cout << "============ Thread ===========\n";
  std::cout << "=== Test Starting ===\n\n";
  Thread thread1("Thread1", myFunction1);
  Thread thread2("Thread2", myFunction2);

  thread1.start();
  thread2.start();

  thread1.stop();
  thread2.stop();
  {
    Thread thread1("Thread1", myFunction1);
    Thread thread2("Thread2", myFunction2);

    thread1.start();
    thread2.start();
    thread1.stop();
    thread2.stop();
  }
}

void workers_pool_test() {
  WorkerPool pool(4);
  auto job = []() {
    threadSafeCout << "Executing job on thread: " << std::this_thread::get_id()
                   << std::endl;
  };

  for (int i = 0; i < 1000; ++i) {
    pool.addJob(job);
  }

  std::this_thread::sleep_for(std::chrono::seconds(2));
  {
    std::cout << "\n\n================================\n";
    std::cout << "============ WorkerPool ===========\n";
    std::cout << "=== 1. Starting ===\n\n";
    WorkerPool pool(4);

    auto job = []() {
      threadSafeCout << "Executing job on thread: "
                     << std::this_thread::get_id() << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    };

    for (int i = 0; i < 1000; ++i) {
      pool.addJob(job);
    }
    std::this_thread::sleep_for(std::chrono::seconds(2));
  }
}

void persistent_worker_test() {
  std::cout << "\n\n================================\n";
  std::cout << "============ PersistentWorker ===========\n";
  std::cout << "=== test Starting ===\n\n";
  PersistentWorker worker;
  auto task1 = []() { threadSafeCout << "Executing Task 1" << std::endl; };
  auto task2 = []() { threadSafeCout << "Executing Task 2" << std::endl; };

  worker.addTask("Task1", task1);
  worker.addTask("Task2", task2);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  worker.removeTask("Task1");
  std::this_thread::sleep_for(std::chrono::seconds(1));
  {
    PersistentWorker worker;
    auto task1 = []() { threadSafeCout << "Executing Task 1" << std::endl; };
    auto task2 = []() { threadSafeCout << "Executing Task 2" << std::endl; };

    worker.addTask("Task1", task1);
    worker.addTask("Task2", task2);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    worker.removeTask("Task1");
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

void message_test() {
  std::cout << "\n\n================================\n";
  std::cout << "============ Message ===========\n";
  std::cout << "=== 1. Starting ===\n\n";

  Message msg(99);
  std::cout << "[INFO] Message created with Type: " << msg.type() << "\n\n";

  uint32_t playerId = 404;
  float x_coord = 125.5f;
  double y_coord = -89.1234;
  bool isCrouching = true;

  std::cout << "--- DATA TO SEND ---\n";
  std::cout << "Player ID: " << playerId << "\n";
  std::cout << "X Coord:   " << x_coord << "\n";
  std::cout << "Y Coord:   " << y_coord << "\n";
  std::cout << "Crouching: " << isCrouching << "\n\n";

  msg << playerId << x_coord << y_coord << isCrouching;

  uint32_t receivedId = 0;
  float receivedX = 0.0f;
  double receivedY = 0.0;
  bool receivedCrouching = false;

  msg >> receivedId >> receivedX >> receivedY >> receivedCrouching;

  std::cout << "--- RECEIVED DATA ---\n";
  std::cout << "Player ID: " << receivedId << "\n";
  std::cout << "X Coord:   " << receivedX << "\n";
  std::cout << "Y Coord:   " << receivedY << "\n";
  std::cout << "Crouching: " << receivedCrouching << "\n\n";

  std::cout << "--- TESTING BUFFER BOUNDARY ---\n";
  int data = 0;
  msg >> data;
}

void server_test() {
  std::cout << "\n\n================================\n";
  std::cout << "============ Client&Server ===========\n";
  std::cout << "=== 1. Starting ===\n\n";

  Server server;

  server.defineAction(1, [&server](long long &clientID, const Message &msg) {
    int value;
    msg >> value;
    std::cout << "[Server] Received an int " << value << " from client "
              << clientID << "\n";

    Message replyMsg(3);
    replyMsg << (value * 2);
    server.sendTo(replyMsg, clientID);
  });

  server.defineAction(2, [](long long &clientID, const Message &msg) {
    size_t length;
    std::string text;
    msg >> length;
    text.reserve(length);
    for (size_t i = 0; i < length; ++i) {
      char c;
      msg >> c;
      text.push_back(c);
    }
    std::cout << "[Server] Received a string '" << text << "' of length "
              << length << " from client " << clientID << "\n";
  });

  server.start(8080);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  Client client;

  client.defineAction(3, [](const Message &msg) {
    int doubledValue;
    msg >> doubledValue;
    std::cout << "[Client] Received a doubled value: " << doubledValue << "\n";
  });

  client.connect("localhost", 8080);

  Message message1(1);
  message1 << 42;
  client.send(message1);

  Message message2(2);
  std::string str = "Hello";
  message2 << str.size();
  for (char c : str) {
    message2 << c;
  }
  client.send(message2);

  bool quit = false;
  while (!quit) {
    server.update();
    client.update();

    std::cout << "\n--- System Updated ---\n";
    std::cout << "Available operations :\n";
    std::cout << " - [Q]uit : close the program\n";
    std::cout << " - Any other input to continue updating\n> ";

    std::string input;
    std::getline(std::cin, input);
    std::transform(input.begin(), input.end(), input.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (input == "quit" || (input.length() == 1 && input[0] == 'q')) {
      quit = true;
    }
  }
  client.disconnect();
}

void ivector2_test() {
  std::cout << "\n\n================================\n";
  std::cout << "============ IVECTOR2 ===========\n";
  std::cout << "=== 1. Starting ===\n\n";

  IVector2<int> vec1(3, 4);
  IVector2<int> vec2(1, 2);

  std::cout << "Vec1 : " << vec1.x << " / " << vec1.y << std::endl;
  std::cout << "Vec2 : " << vec2.x << " / " << vec2.y << std::endl;

  auto vecAdd = vec1 + vec2;
  std::cout << "vec1 + vec2 = (" << vecAdd.x << ", " << vecAdd.y << ")"
            << std::endl;

  auto vecSub = vec1 - vec2;
  std::cout << "vec1 - vec2 = (" << vecSub.x << ", " << vecSub.y << ")"
            << std::endl;

  auto vecMul = vec1 * vec2;
  std::cout << "vec1 * vec2 = (" << vecMul.x << ", " << vecMul.y << ")"
            << std::endl;

  auto vecDiv = vec1 / vec2;
  std::cout << "vec1 / vec2 = (" << vecDiv.x << ", " << vecDiv.y << ")"
            << std::endl;

  bool isEqual = vec1 == vec2;
  std::cout << "vec1 == vec2: " << (isEqual ? "true" : "false") << std::endl;

  bool isNotEqual = vec1 != vec2;
  std::cout << "vec1 != vec2: " << (isNotEqual ? "true" : "false") << std::endl;

  float len = vec1.length();
  std::cout << "Length of vec1: " << len << std::endl;

  auto normVec = vec1.normalize();
  std::cout << "Normalized vec1 = (" << normVec.x << ", " << normVec.y << ")"
            << std::endl;

  float dotProd = vec1.dot(vec2);
  std::cout << "Dot product of vec1 and vec2: " << dotProd << std::endl;

  auto crossProd = vec1.cross();
  std::cout << "Cross product of vec1: (" << crossProd.x << ", " << crossProd.y
            << ")" << std::endl;

  // what about the 0 vector?
  IVector2<int> vec3(0, 0);
  // test the default constructor
  IVector2<int> vec4{};
  auto normVec3 = vec3.normalize();
  std::cout << "Normalized zero vec = (" << normVec3.x << ", " << normVec3.y
            << ")" << std::endl;
  crossProd = vec3.cross();
  std::cout << "Cross product of zero vec: (" << crossProd.x << ", "
            << crossProd.y << ")" << std::endl;
  dotProd = vec3.dot(vec3);
  std::cout << "Dot product of zero vec: " << dotProd << std::endl;
  len = vec3.length();
  std::cout << "Length of zero vec: " << len << std::endl;
}

void ivector3_test() {
  std::cout << "\n\n================================\n";
  std::cout << "============ IVECTOR3 ===========\n";
  std::cout << "=== 1. Starting ===\n\n";
  {
    IVector3<int> vec1(3, 4, 1);
    IVector3<int> vec2(1, 2, 3);
    IVector3<int> zeroVec{};
    std::cout << "Vec1 : " << vec1.x << " / " << vec1.y << " / " << vec1.z
              << std::endl;
    std::cout << "Vec2 : " << vec2.x << " / " << vec2.y << " / " << vec2.z
              << std::endl;

    auto vecAdd = vec1 + vec2;
    std::cout << "vec1 + vec2 = (" << vecAdd.x << ", " << vecAdd.y << ", "
              << vecAdd.z << ")" << std::endl;

    auto vecSub = vec1 - vec2;
    std::cout << "vec1 - vec2 = (" << vecSub.x << ", " << vecSub.y << ", "
              << vecSub.z << ")" << std::endl;

    auto vecMul = vec1 * vec2;
    std::cout << "vec1 * vec2 = (" << vecMul.x << ", " << vecMul.y << ", "
              << vecMul.z << ")" << std::endl;

    auto vecDiv = vec1 / vec2;
    std::cout << "vec1 / vec2 = (" << vecDiv.x << ", " << vecDiv.y << ", "
              << vecDiv.z << ")" << std::endl;

    bool isEqual = vec1 == vec2;
    std::cout << "vec1 == vec2: " << (isEqual ? "true" : "false") << std::endl;

    bool isNotEqual = vec1 != vec2;
    std::cout << "vec1 != vec2: " << (isNotEqual ? "true" : "false")
              << std::endl;

    float len = vec1.length();
    std::cout << "Length of vec1: " << len << std::endl;

    auto normVec = vec1.normalize();
    std::cout << "Normalized vec1 = (" << normVec.x << ", " << normVec.y << ", "
              << normVec.z << ")" << std::endl;

    float dotProd = vec1.dot(vec2);
    std::cout << "Dot product of vec1 and vec2: " << dotProd << std::endl;

    auto crossProd = vec1.cross(vec2);
    std::cout << "Cross product of vec1 and vec2: (" << crossProd.x << ", "
              << crossProd.y << ", " << crossProd.z << ")" << std::endl;
    len = zeroVec.length();
    std::cout << "Length of vec1: " << len << std::endl;

    normVec = zeroVec.normalize();
    std::cout << "Normalized vec1 = (" << normVec.x << ", " << normVec.y << ", "
              << normVec.z << ")" << std::endl;

    dotProd = zeroVec.dot(vec2);
    std::cout << "Dot product of vec1 and vec2: " << dotProd << std::endl;

    crossProd = zeroVec.cross(vec2);
    std::cout << "Cross product of vec1 and vec2: (" << crossProd.x << ", "
              << crossProd.y << ", " << crossProd.z << ")" << std::endl;
  }
}

void random_2D_coordinate_generator_test() {
  std::cout << "\n\n================================\n";
  std::cout << "============ Random2DCoordinateGenerator ===========\n";
  std::cout << "=== 1. Starting ===\n\n";
  Random2DCoordinateGenerator randomGenerator(1);

  std::vector<std::pair<long long, long long>> coordinates = {
      {5, 3}, {7, 2}, {1, 9}, {0, 0}};
  std::vector<long long> firstGenerated;

  std::cout << "First round of generation:" << std::endl;
  for (const auto &coord : coordinates) {
    long long x = coord.first;
    long long y = coord.second;
    long long randomNumber = randomGenerator(x, y);
    firstGenerated.push_back(randomNumber);
    std::cout << "Random number using coordinates (" << x << ", " << y
              << "): " << randomNumber << std::endl;
  }
  std::cout << std::endl;

  std::cout << "Second round of generation:" << std::endl;
  for (size_t i = 0; i < coordinates.size(); ++i) {
    long long x = coordinates[i].first;
    long long y = coordinates[i].second;
    long long randomNumber = randomGenerator(x, y);

    std::cout << "Random number using coordinates (" << x << ", " << y
              << "): " << randomNumber << std::endl;

    if (randomNumber == firstGenerated[i]) {
      std::cout << "  => Matches the previous generated value. Consistent!"
                << std::endl;
    } else {
      std::cout
          << "  => Does not match the previous generated value. Inconsistent!"
          << std::endl;
    }
  }
  std::cout << std::endl;
}

void perlin_noise2D_test() {
  std::cout << "\n\n================================\n";
  std::cout << "============ PerlinNoise2D ===========\n";
  std::cout << "=== 1. Starting ===\n\n";
  PerlinNoise2D perlin;

  const int gridSize = 40;
  char visualChars[] = {' ', '.', ':', '-', '=', '+', '*', '#', '%', '@'};

  std::cout << "Sampling 2D Perlin noise over a " << gridSize << "x" << gridSize
            << " grid:" << std::endl
            << std::endl;

  for (int y = 0; y < gridSize; ++y) {
    for (int x = 0; x < gridSize; ++x) {
      float sample = perlin.sample(x * 0.3f, y * 0.3f);
      sample = (sample + 1) / 2;
      int charIndex = std::round(sample * 9);

      std::cout << visualChars[charIndex] << " ";
    }
    std::cout << std::endl;
  }
}

void ppm_image_exporter_test() {
  std::cout << "\n\n================================\n";
  std::cout << "============ PPMImageExporter ===========\n";
  std::cout << "=== 1. Starting ===\n\n";
  PerlinNoise2D perlin(42);

  PPMImageExporter exporter("terrain_map.ppm", 500, 500);
  exporter.generateTerrain(perlin, 0.03f);
}

void observable_value_test() {
  std::cout << "\n\n================================\n";
  std::cout << "============ ObservableValue ===========\n";
  std::cout << "--- Testing ObservableValue ---" << std::endl;
  {
    ObservableValue<int> playerHealth(100);

    playerHealth.subscribe([](const int &newHealth) {
      std::cout << "[UI System] Health Bar updated to: " << newHealth << " HP"
                << std::endl;
    });

    playerHealth.subscribe([](const int &newHealth) {
      if (newHealth <= 20) {
        std::cout
            << "[Audio System] PLAYING DANGER SIREN! Heartbeat sound fast!"
            << std::endl;
      }
    });

    std::cout << "\nPlayer takes 10 damage..." << std::endl;
    playerHealth.set(90);

    std::cout << "\nPlayer finds a small potion..." << std::endl;
    playerHealth = 95;

    std::cout << "\nPlayer steps on a massive trap!" << std::endl;
    playerHealth = 15;
  }
}

void timer_test() {
  std::cout << "\n\n================================\n";
  std::cout << "============ Timer ===========\n" << std::endl;

  Timer myTimer(1000);
  std::cout << "Timer set. Waiting for timeout..." << std::endl;

  while (!myTimer.hasTimedOut()) {
    std::cout << "." << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  std::cout << "BEEP! Timer has timed out!" << std::endl;
}

void chronometer_test() {
  std::cout << "\n\n================================\n";
  std::cout << "============ chronometer ===========\n" << std::endl;
  Chronometer chrono;

  chrono.start();
  std::cout << "Chronometer started. Doing some 'heavy' work..." << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(1500));

  chrono.stop();
  std::cout << "Work finished!" << std::endl;
  std::cout << "Elapsed time: " << chrono.getElapsedSeconds()
            << " seconds.\n\n";
}

void command_pattern_test() {
  std::cout << "\n\n================================\n";
  std::cout << "============ Templated Command Pattern ===========\n"
            << std::endl;

  PlayerReceiver myPlayer;
  AudioSystemReceiver myAudio;
  std::vector<std::unique_ptr<Command>> commandQueue;

  commandQueue.push_back(std::make_unique<SimpleCommand<PlayerReceiver>>(
      &myPlayer, &PlayerReceiver::jump));
  commandQueue.push_back(std::make_unique<SimpleCommand<AudioSystemReceiver>>(
      &myAudio, &AudioSystemReceiver::playJumpSound));

  std::cout << "Executing queue...\n\n";
  for (const auto &cmd : commandQueue) {
    cmd->execute();
  }
  std::cout << "\n=== Testing Null Receiver Edge Case ===\n";
  std::cout << "Creating a SimpleCommand with a nullptr receiver...\n";

  // Pass nullptr instead of &myPlayer
  auto nullSimpleCmd = std::make_unique<SimpleCommand<PlayerReceiver>>(
      nullptr, &PlayerReceiver::jump);

  std::cout << "Executing null simple command...\n";
  nullSimpleCmd->execute(); // Should safely bypass thanks to the if-check!
  std::cout << "  -> Success! Safe execution, no crash.\n";
}

void lambda_command_test() {
  std::cout << "\n\n================================\n";
  std::cout << "============= Lambda Command Queue ============\n" << std::endl;

  PlayerReceiver myPlayer;
  AudioSystemReceiver myAudio;
  std::vector<std::unique_ptr<Command>> commandQueue;

  commandQueue.push_back(
      std::make_unique<LambdaCommand>([&myPlayer]() { myPlayer.jump(); }));
  commandQueue.push_back(std::make_unique<LambdaCommand>(
      [&myAudio]() { myAudio.playJumpSound(); }));

  int damage = 50;
  commandQueue.push_back(std::make_unique<LambdaCommand>([damage]() {
    std::cout << "[System] Dealt " << damage << " damage using a lambda!"
              << std::endl;
  }));

  std::cout << "Executing lambda queue...\n\n";
  for (const auto &cmd : commandQueue) {
    cmd->execute();
  }
}

// this is for the bonus command design pattern
void mixed_signature_test() {
  GameEngine engine;
  std::vector<std::unique_ptr<Command>> queue;

  queue.push_back(
      std::make_unique<LambdaCommand>([&engine]() { engine.saveGame(); }));
  queue.push_back(std::make_unique<LambdaCommand>(
      [&engine]() { engine.playMusic("boss_theme.mp3", 0.8f); }));

  queue.push_back(std::make_unique<LambdaCommand>([&engine]() {
    int result = engine.calculateDamage(50);
    if (result > 90)
      std::cout << "Critical Hit!\n";
  }));

  std::cout << "Executing Mixed Queue...\n";
  for (const auto &cmd : queue) {
    cmd->execute();
  }
}