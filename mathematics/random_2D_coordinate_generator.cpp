#include "random_2D_coordinate_generator.hpp"

Random2DCoordinateGenerator::Random2DCoordinateGenerator(long long seed)
    : _seed(seed) {}
Random2DCoordinateGenerator::~Random2DCoordinateGenerator() {}

long long Random2DCoordinateGenerator::seed() const { return _seed; }

// given the same seed and coordinates I will have the same result
long long Random2DCoordinateGenerator::operator()(const long long &x,
                                                  const long long &y) const {
  // Multiplying inputs by large prime numbers ensures that the bits wrap around
  // the 64-bit limit in highly irregular ways, which breaks up linear patterns
  const long long PRIME_X = 668265263LL;
  const long long PRIME_Y = 374761393LL;
  const long long PRIME_SEED = 1274126177LL;

  long long hash = (x * PRIME_X) ^ (y * PRIME_Y) ^ (_seed * PRIME_SEED);
  // the 64bit finalization mix(fmix64) from MurmurHash3, created by
  // Austin Appleby, and are also famously used in the SplitMix64
  // random number generator.
  hash ^= hash >> 33;
  hash *= 0xff51afd7ed558ccdLL; // Another large prime multiplier
  hash ^= hash >> 33;
  hash *= 0xc4ceb9fe1a85ec53LL;
  hash ^= hash >> 33;

  return hash;
}