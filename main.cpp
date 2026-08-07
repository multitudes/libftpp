#include "libftpp.hpp"
#include "main_tests.hpp"
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

// =============================================================================
// Main Test
// =============================================================================
int main() {
  pool_test();
  databuffer_test();
  memento_test();
  observer_test();
  singleton_test();
  state_machine_test();

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

  // threadSafeCout.setPrefix("[Main Thread] ");
  // int userAge = 0;

  // Uncomment this line below if you want to test the interactive input!
  // threadSafeCout.prompt("Enter your age to exit: ", userAge);

  // threadSafeCout << "Test complete. User age entered: " << userAge <<
  // std::endl;

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

  // --------------------- Message ---------------------
  std::cout << "\n\n================================\n";
  std::cout << "============ Message ===========\n";
  std::cout << "=== 1. Starting ===\n\n";
  // 1. Create a message of Type 99 (e.g., "Player Position Update")
  Message msg(99);
  std::cout << "[INFO] Message created with Type: " << msg.type() << "\n\n";

  // 2. Create some trivially copyable variables to send
  uint32_t playerId = 404;
  float x_coord = 125.5f;
  double y_coord = -89.1234;
  bool isCrouching = true;

  std::cout << "--- DATA TO SEND ---\n";
  std::cout << "Player ID: " << playerId << "\n";
  std::cout << "X Coord:   " << x_coord << "\n";
  std::cout << "Y Coord:   " << y_coord << "\n";
  std::cout << "Crouching: " << isCrouching << "\n\n";

  // 3. Serialize (Pack the envelope)
  // Thanks to returning `*this`, we can chain them together!
  msg << playerId << x_coord << y_coord << isCrouching;

  // 4. Create empty variables to hold the incoming data on the "Server" side
  uint32_t receivedId = 0;
  float receivedX = 0.0f;
  double receivedY = 0.0;
  bool receivedCrouching = false;

  // 5. Deserialize (Unpack the envelope)
  // CRITICAL: You must unpack in the EXACT same order you packed!
  msg >> receivedId >> receivedX >> receivedY >> receivedCrouching;

  std::cout << "--- RECEIVED DATA ---\n";
  std::cout << "Player ID: " << receivedId << "\n";
  std::cout << "X Coord:   " << receivedX << "\n";
  std::cout << "Y Coord:   " << receivedY << "\n";
  std::cout << "Crouching: " << receivedCrouching << "\n\n";

  // 6. Test the buffer safety check!
  // We already read all the bytes. If we try to read one more integer,
  // it should fail gracefully and print your error message instead of crashing.
  std::cout << "--- TESTING BUFFER BOUNDARY ---\n";
  int data = 0;
  msg >> data;

  // --------------------- Client&Server ---------------------
  std::cout << "\n\n================================\n";
  std::cout << "============ Client&Server ===========\n";
  std::cout << "=== 1. Starting ===\n\n";
  // ---------------------------------------------------------
  // 1. SETUP AND START THE SERVER
  // ---------------------------------------------------------
  Server server;

  server.defineAction(1, [&server](long long &clientID, const Message &msg) {
    int value;
    msg >> value;
    std::cout << "[Server] Received an int " << value << " from client "
              << clientID << "\n";

    // Send back a message of type 3 with double the value
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

  // Give the server thread a tiny fraction of a second to bind the port
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // ---------------------------------------------------------
  // 2. SETUP AND CONNECT THE CLIENT
  // ---------------------------------------------------------
  Client client;

  client.defineAction(3, [](const Message &msg) {
    int doubledValue;
    msg >> doubledValue;
    std::cout << "[Client] Received a doubled value: " << doubledValue << "\n";
  });

  client.connect("localhost", 8080);

  // ---------------------------------------------------------
  // 3. SEND TEST MESSAGES
  // ---------------------------------------------------------
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

  // ---------------------------------------------------------
  // 4. UNIFIED UPDATE LOOP
  // ---------------------------------------------------------
  bool quit = false;
  while (!quit) {
    // Update both the server and the client!
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

  // --------------------- IVECTOR2 ---------------------
  std::cout << "\n\n================================\n";
  std::cout << "============ IVECTOR2 ===========\n";
  std::cout << "=== 1. Starting ===\n\n";

  IVector2<int> vec1(3, 4);
  IVector2<int> vec2(1, 2);

  std::cout << "Vec1 : " << vec1.x << " / " << vec1.y << std::endl;
  std::cout << "Vec2 : " << vec2.x << " / " << vec2.y << std::endl;

  // Test operator overloads
  auto vecAdd = vec1 + vec2;
  std::cout << "vec1 + vec2 = (" << vecAdd.x << ", " << vecAdd.y << ")"
            << std::endl;
  // Expected: vec1 + vec2 = (4, 6)

  auto vecSub = vec1 - vec2;
  std::cout << "vec1 - vec2 = (" << vecSub.x << ", " << vecSub.y << ")"
            << std::endl;
  // Expected: vec1 - vec2 = (2, 2)

  auto vecMul = vec1 * vec2;
  std::cout << "vec1 * vec2 = (" << vecMul.x << ", " << vecMul.y << ")"
            << std::endl;
  // Expected: vec1 * vec2 = (3, 8)

  auto vecDiv = vec1 / vec2;
  std::cout << "vec1 / vec2 = (" << vecDiv.x << ", " << vecDiv.y << ")"
            << std::endl;
  // Expected: vec1 / vec2 = (3, 2)

  bool isEqual = vec1 == vec2;
  std::cout << "vec1 == vec2: " << (isEqual ? "true" : "false") << ""
            << std::endl;
  // Expected: vec1 == vec2: false

  bool isNotEqual = vec1 != vec2;
  std::cout << "vec1 != vec2: " << (isNotEqual ? "true" : "false") << ""
            << std::endl;
  // Expected: vec1 != vec2: true

  // Test additional methods
  float len = vec1.length();
  std::cout << "Length of vec1: " << len << "" << std::endl;
  // Expected: Length of vec1: 5 (or sqrt(3*3 + 4*4))

  auto normVec = vec1.normalize();
  std::cout << "Normalized vec1 = (" << normVec.x << ", " << normVec.y << ")"
            << std::endl;
  // Expected: Normalized vec1 = (0.6, 0.8)

  float dotProd = vec1.dot(vec2);
  std::cout << "Dot product of vec1 and vec2: " << dotProd << "" << std::endl;
  // Expected: Dot product of vec1 and vec2: 11 (or 3*1 + 4*2)

  auto crossProd = vec1.cross();
  std::cout << "Cross product of vec1: (" << crossProd.x << ", " << crossProd.y
            << ")" << std::endl;
  // Expected: Cross product of vec1: (some_value, some_value)

  // --------------------- IVECTOR3 ---------------------
  std::cout << "\n\n================================\n";
  std::cout << "============ IVECTOR3 ===========\n";
  std::cout << "=== 1. Starting ===\n\n";
  {

    IVector3<int> vec1(3, 4, 1);
    IVector3<int> vec2(1, 2, 3);

    std::cout << "Vec1 : " << vec1.x << " / " << vec1.y << " / " << vec1.z
              << std::endl;
    std::cout << "Vec2 : " << vec2.x << " / " << vec2.y << " / " << vec2.z
              << std::endl;

    // Test operator overloads
    auto vecAdd = vec1 + vec2;
    std::cout << "vec1 + vec2 = (" << vecAdd.x << ", " << vecAdd.y << ", "
              << vecAdd.z << ")" << std::endl;
    // Expected: vec1 + vec2 = (4, 6, 4)

    auto vecSub = vec1 - vec2;
    std::cout << "vec1 - vec2 = (" << vecSub.x << ", " << vecSub.y << ", "
              << vecSub.z << ")" << std::endl;
    // Expected: vec1 - vec2 = (2, 2, -2)

    auto vecMul = vec1 * vec2;
    std::cout << "vec1 * vec2 = (" << vecMul.x << ", " << vecMul.y << ", "
              << vecMul.z << ")" << std::endl;
    // Expected: vec1 * vec2 = (3, 8, 3)

    auto vecDiv = vec1 / vec2;
    std::cout << "vec1 / vec2 = (" << vecDiv.x << ", " << vecDiv.y << ", "
              << vecDiv.z << ")" << std::endl;
    // Expected: vec1 / vec2 = (3, 2, 0)

    bool isEqual = vec1 == vec2;
    std::cout << "vec1 == vec2: " << (isEqual ? "true" : "false") << std::endl;
    // Expected: vec1 == vec2: false

    bool isNotEqual = vec1 != vec2;
    std::cout << "vec1 != vec2: " << (isNotEqual ? "true" : "false")
              << std::endl;
    // Expected: vec1 != vec2: true

    // Test additional methods
    float len = vec1.length();
    std::cout << "Length of vec1: " << len << std::endl;
    // Expected: Length of vec1: 5.099 (or sqrt(3*3 + 4*4 + 1*1))

    auto normVec = vec1.normalize();
    std::cout << "Normalized vec1 = (" << normVec.x << ", " << normVec.y << ", "
              << normVec.z << ")" << std::endl;
    // Expected: Normalized vec1 = (some_value, some_value, some_value)

    float dotProd = vec1.dot(vec2);
    std::cout << "Dot product of vec1 and vec2: " << dotProd << std::endl;
    // Expected: Dot product of vec1 and vec2: 14 (or 3*1 + 4*2 + 1*3)

    auto crossProd = vec1.cross(vec2);
    std::cout << "Cross product of vec1 and vec2: (" << crossProd.x << ", "
              << crossProd.y << ", " << crossProd.z << ")" << std::endl;
    // Expected: Cross product of vec1 and vec2: (some_value, some_value,
    // some_value)
  }

  // --------------------- Random2DCoordinateGenerator ---------------------
  std::cout << "\n\n================================\n";
  std::cout << "============ Random2DCoordinateGenerator ===========\n";
  std::cout << "=== 1. Starting ===\n\n";
  Random2DCoordinateGenerator randomGenerator(1);

  // Store a list of coordinates to test
  std::vector<std::pair<long long, long long>> coordinates = {
      {5, 3}, {7, 2}, {1, 9}, {0, 0}};

  // Store the random numbers generated the first time
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

    // Check if the number is the same as generated the first time
    if (randomNumber == firstGenerated[i]) {
      std::cout << "  => Matches the previous generated value. Consistent!"
                << std::endl; // Expected: Should always match
    } else {
      std::cout
          << "  => Does not match the previous generated value. Inconsistent!"
          << std::endl;
    }
  }
  std::cout << std::endl;

  // --------------------- PerlinNoise2D ---------------------
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
      float sample =
          perlin.sample(x * 0.3f, y * 0.3f);  // Adjust these factors as needed
      sample = (sample + 1) / 2;              // Map from [-1, 1] to [0, 1]
      int charIndex = std::round(sample * 9); // Map from [0, 1] to [0, 9]

      std::cout << visualChars[charIndex] << " ";
    }
    std::cout << std::endl;
  }
  {
    // --------------------- PPMImageExporter ---------------------
    std::cout << "\n\n================================\n";
    std::cout << "============ PPMImageExporter ===========\n";
    std::cout << "=== 1. Starting ===\n\n";
    PerlinNoise2D perlin(42);

    // Create an exporter for a 500x500 pixel image
    PPMImageExporter exporter("terrain_map.ppm", 500, 500);

    // Generate it!
    exporter.generateTerrain(perlin, 0.03f);
  }

  // --------------------- ObservableValue ---------------------
  std::cout << "\n\n================================\n";
  std::cout << "============ ObservableValue ===========\n";
  std::cout << "--- Testing ObservableValue ---" << std::endl;
  {

    // 1. Create our observable variable starting at 100
    ObservableValue<int> playerHealth(100);

    // 2. Subscribe the UI System
    // (Using a lambda function for a quick, inline callback)
    playerHealth.subscribe([](const int &newHealth) {
      std::cout << "[UI System] Health Bar updated to: " << newHealth << " HP"
                << std::endl;
    });

    // 3. Subscribe the Audio System
    // (It only reacts if health drops dangerously low)
    playerHealth.subscribe([](const int &newHealth) {
      if (newHealth <= 20) {
        std::cout
            << "[Audio System] PLAYING DANGER SIREN! Heartbeat sound fast!"
            << std::endl;
      }
    });

    std::cout << "\nPlayer takes 10 damage..." << std::endl;
    playerHealth.set(90); // Triggers UI update

    std::cout << "\nPlayer finds a small potion..." << std::endl;
    playerHealth = 95; // Uses our overloaded operator to trigger UI update

    std::cout << "\nPlayer steps on a massive trap!" << std::endl;
    playerHealth = 15; // Triggers BOTH the UI update AND the Audio siren!
  }
  std::cout << "\n\n================================\n";
  std::cout << "============ ENDING ===========\n";
  return 0;
}
