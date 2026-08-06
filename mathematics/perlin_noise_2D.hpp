#pragma once

#include "random_2D_coordinate_generator.hpp"

struct PerlinNoise2D {
  Random2DCoordinateGenerator _generator;

  // Pass the seed down to the internal generator!
  PerlinNoise2D(long long seed = 42);
  ~PerlinNoise2D();
  // Returns a perlin noise value for the provided coordinates.
  float sample(float x, float y) const;
  float operator()(const float &x, const float &y) const;
};