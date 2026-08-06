#include "ppm_image_exporter.hpp"
#include <fstream>
#include <iostream>

// Constructor initializes the target file and dimensions
PPMImageExporter::PPMImageExporter(const std::string &filename, int width,
                                   int height)
    : _filename(filename), _width(width), _height(height) {}

PPMImageExporter::~PPMImageExporter() {}

void PPMImageExporter::writeColor(std::ofstream &file, float height) const {
  int r = 0, g = 0, b = 0;

  if (height < 0.35f) {
    // Deep Water
    r = 30;
    g = 100;
    b = 200;
  } else if (height < 0.45f) {
    // Shallow Water
    r = 50;
    g = 150;
    b = 255;
  } else if (height < 0.50f) {
    // Sand
    r = 238;
    g = 214;
    b = 175;
  } else if (height < 0.70f) {
    // Grass / Forest
    r = 34;
    g = 139;
    b = 34;
  } else if (height < 0.85f) {
    // Mountain Rock
    r = 120;
    g = 120;
    b = 120;
  } else {
    // Snow Peaks
    r = 255;
    g = 255;
    b = 255;
  }

  // PPM expects RGB numbers separated by spaces
  file << r << " " << g << " " << b << "\n";
}

void PPMImageExporter::generateTerrain(const PerlinNoise2D &noise,
                                       float zoom) const {
  // 1. Open the file
  std::ofstream file(_filename);
  if (!file.is_open()) {
    std::cerr << "Error: Could not create file " << _filename << std::endl;
    return;
  }

  // 2. Write the required PPM header
  // "P3" means this is an ASCII RGB image
  file << "P3\n";
  file << _width << " " << _height << "\n";
  file << "255\n"; // 255 is the maximum color value

  // 3. Loop through every pixel
  for (int y = 0; y < _height; ++y) {
    for (int x = 0; x < _width; ++x) {
      // Get the noise value
      float noiseVal = noise.sample(x * zoom, y * zoom);

      // Map from roughly [-1.0, 1.0] to [0.0, 1.0]
      noiseVal = (noiseVal + 1.0f) / 2.0f;

      // Safety clamp just in case
      if (noiseVal < 0.0f)
        noiseVal = 0.0f;
      if (noiseVal > 1.0f)
        noiseVal = 1.0f;

      // Write the color for this pixel
      writeColor(file, noiseVal);
    }
  }

  file.close();
  std::cout << "Successfully exported terrain map to " << _filename
            << std::endl;
}