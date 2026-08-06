#pragma once

struct Random2DCoordinateGenerator {
  // adding = 42 as default makes this a default constructor
  Random2DCoordinateGenerator(long long seed = 42);
  ~Random2DCoordinateGenerator();
  long long _seed;
  long long seed() const;
  long long operator()(const long long &x, const long long &y) const;
};
