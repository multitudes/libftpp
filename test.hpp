#pragma once

#include <iostream>
#include <string>

#include "libftpp.hpp"

// =============================================================================
// ENUMS
// =============================================================================
enum EnemyState { IDLE, CHASE, ATTACK };
enum class EventType { EVENT_ONE, EVENT_TWO, EVENT_THREE };
enum class State { Idle, Running, Paused, Stopped };

// =============================================================================
// DUMMY CLASSES & STRUCTS
// =============================================================================

class Particle {
public:
  std::string name;
  float x, y, z;

  Particle(std::string n, float startX, float startY, float startZ)
      : name(n), x(startX), y(startY), z(startZ) {
    std::cout << "  [+] Particle '" << name << "' constructed.\n";
  }

  ~Particle() { std::cout << "  [-] Particle '" << name << "' destroyed.\n"; }

  void printPosition() {
    std::cout << "      " << name << " is at (" << x << ", " << y << ", " << z
              << ")\n";
  }
};

class Player : public Memento {
private:
  friend class Memento;
  int _health;
  float _x;
  float _y;

  void _saveToSnapshot(Memento::Snapshot &snapshot) const override {
    snapshot << _health << _x << _y;
  }

  void _loadFromSnapshot(Memento::Snapshot &snapshot) override {
    snapshot >> _health >> _x >> _y;
  }

public:
  Player(int hp, float startX, float startY)
      : _health(hp), _x(startX), _y(startY) {}

  void takeDamage(int amount) {
    _health -= amount;
    if (_health < 0)
      _health = 0;
  }

  void move(float dx, float dy) {
    _x += dx;
    _y += dy;
  }

  void printStatus() const {
    std::cout << "  -> Health: " << _health << " | Position: (" << _x << ", "
              << _y << ")\n";
  }
};

struct PlayerEvent {
  int eventType;
  std::string playerName;

  bool operator<(const PlayerEvent &other) const {
    if (eventType != other.eventType) {
      return eventType < other.eventType;
    }
    return playerName < other.playerName;
  }
};

class GameManager {
private:
  friend class Singleton<GameManager>;
  int _currentLevel;
  std::string _difficulty;

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

class MyClass {
public:
  MyClass(int value) {
    std::cout << "MyClass constructor, with value [" << value << "]"
              << std::endl;
  }
  void printMessage() { std::cout << "Hello from MyClass" << std::endl; }
};

class PlayerReceiver {
public:
  void jump() { std::cout << "[Player] Jumped into the air!" << std::endl; }
};

class AudioSystemReceiver {
public:
  void playJumpSound() {
    std::cout << "[Audio] PLAYING: 'boing.wav'" << std::endl;
  }
};

class GameEngine {
public:
  void saveGame() { std::cout << "Game saved." << std::endl; }
  void playMusic(std::string track, float volume) {
    std::cout << "Playing " << track << " at volume " << volume << std::endl;
  }
  int calculateDamage(int baseDamage) {
    std::cout << "Calculated damage: " << baseDamage * 2 << std::endl;
    return baseDamage * 2;
  }
};

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

std::string stateToString(EnemyState state);
void workerTask(int threadID, std::string taskName);
void pool_test();
void databuffer_test();
void memento_test();
void observer_test();
void singleton_test();
void state_machine_test();
void printNumbers(const std::string &p_prefix);
void thread_safe_iostream_test();

// Forward declaration needed if ThreadSafeQueue is not included above
template <typename T> class ThreadSafeQueue;
void testPush(ThreadSafeQueue<int> &p_queue, int p_value);
void testPop(ThreadSafeQueue<int> &p_queue);

void thread_safe_queue_test();
void myFunction1();
void myFunction2();
void thread_test();
void workers_pool_test();
void persistent_worker_test();
void message_test();
void server_test();
void ivector2_test();
void ivector3_test();
void random_2D_coordinate_generator_test();
void perlin_noise2D_test();
void ppm_image_exporter_test();
void observable_value_test();
void timer_test();
void chronometer_test();
void command_pattern_test();
void lambda_command_test();
void mixed_signature_test();