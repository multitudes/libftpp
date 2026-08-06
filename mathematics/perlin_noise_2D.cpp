#include "perlin_noise_2D.hpp"
#include "ivector2.hpp"
#include "random_2D_coordinate_generator.hpp"
#include <cmath>

PerlinNoise2D::PerlinNoise2D(long long seed) {
  Random2DCoordinateGenerator _generator = Random2DCoordinateGenerator(seed);
}

PerlinNoise2D ::~PerlinNoise2D() {}

// Returns a perlin noise value for the provided coordinates.
float PerlinNoise2D::sample(float x, float y) const {}
float PerlinNoise2D::operator()(const float &x, const float &y) const {
  long long X = static_cast<long long>(std::floor(x));
  long long Y = static_cast<long long>(std::floor(y));

  IVector2<float> orig_point(x, y);

  // The 4 integer corners of our grid cell to feed into the generator
  IVector2<long long> bottom_left(X, Y);
  IVector2<long long> bottom_right(X + 1, Y);
  IVector2<long long> upper_left(X, Y + 1);
  IVector2<long long> upper_right(X + 1, Y + 1);

  // Where our point sits inside the cell (values from 0.0 to 1.0)
  IVector2<float> local_coord = orig_point - IVector2<float>(X, Y);

  // We need four float vectors pointing from each of those four corners to the
  // original point. This is to get the dot products
  IVector2<float> first = local_coord; // from bottom_left
  IVector2<float> second =
      orig_point - IVector2<float>(X + 1, Y); // from bottom_right
  IVector2<float> third =
      orig_point - IVector2<float>(X, Y + 1); // from upper_left
  IVector2<float> fourth =
      orig_point - IVector2<float>(X + 1, Y + 1); // from upper_right

  // get the angles
  float bottom_left_angle =
      (_generator(bottom_left.x, bottom_left.y) % 360) * 3.14159265f / 180.0f;
  float bottom_right_angle =
      (_generator(bottom_left.x, bottom_left.y) % 360) * 3.14159265f / 180.0f;
  float upper_left_angle =
      (_generator(bottom_left.x, bottom_left.y) % 360) * 3.14159265f / 180.0f;
  float upper_right_angle =
      (_generator(bottom_left.x, bottom_left.y) % 360) * 3.14159265f / 180.0f;
  // get the random direction vectors
  IVector2<float> bottom_left_gradient(std::cos(bottom_left_angle),
                                       std::sin(bottom_left_angle));
  IVector2<float> bottom_right_gradient(std::cos(bottom_right_angle),
                                        std::sin(bottom_right_angle));
  IVector2<float> upper_left_gradient(std::cos(upper_left_angle),
                                      std::sin(upper_left_angle));
  IVector2<float> upper_right_gradient(std::cos(upper_right_angle),
                                       std::sin(upper_right_angle));
  // get the dot product by comparing that random arrow to the distance vector
  // we calculated earlier(first)
  float bottom_left_dot = bottom_left_gradient.dot(first);
  float bottom_right_dot = bottom_right_gradient.dot(first);
  float upper_left_dot = upper_left_gradient.dot(first);
  float upper_right_dot = bottom_right_gradient.dot(first);
}
