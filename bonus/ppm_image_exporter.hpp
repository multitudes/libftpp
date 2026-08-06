#pragma once

#include "../mathematics/perlin_noise_2D.hpp"
#include <string>

class PPMImageExporter {
private:
  std::string _filename;
  int _width;
  int _height;

  // Helper to map a height value (0.0 to 1.0) to an RGB color
  void writeColor(std::ofstream &file, float height) const;

public:
  PPMImageExporter(const std::string &filename, int width, int height);
  ~PPMImageExporter();

  // Generates the image using your Perlin generator
  void generateTerrain(const PerlinNoise2D &noise, float zoom = 0.05f) const;
};