#pragma once

#include <cmath>

template <typename TType> struct IVector3 {
  TType x;
  TType y;
  TType z;

  IVector3<TType>(TType p_x = 0, TType p_y = 0, TType p_z = 0)
      : x(p_x), y(p_y), z(p_z) {}
  ~IVector3<TType>() {}

  // Operator Overloads
  IVector3<TType> operator+(const IVector3<TType> &other) const {
    return IVector3<TType>(x + other.x, y + other.y, z + other.z);
  }

  IVector3<TType> operator-(const IVector3<TType> &other) const {
    return IVector3<TType>(x - other.x, y - other.y, z - other.z);
  }

  IVector3<TType> operator*(const IVector3<TType> &other) const {
    return IVector3<TType>(x * other.x, y * other.y, z * other.z);
  }

  IVector3<TType> operator/(const IVector3<TType> &other) const {
    return IVector3<TType>(x / other.x, y / other.y, z / other.z);
  }

  bool operator==(const IVector3<TType> &other) const {
    return x == other.x && y == other.y && z == other.z;
  }

  bool operator!=(const IVector3<TType> &other) const {
    return x != other.x || y != other.y || z != other.z;
  }

  // Mathematics
  float length() const { return std::sqrt(x * x + y * y + z * z); }

  IVector3<float> normalize() const {
    float len = length();
    if (len == 0.0f) {
      return IVector3<float>(0.0f, 0.0f, 0.0f);
    }

    return IVector3<float>(static_cast<float>(x) / len,
                           static_cast<float>(y) / len,
                           static_cast<float>(z) / len);
  }

  float dot(const IVector3<TType> &other) const {
    return static_cast<float>(x * other.x + y * other.y + z * other.z);
  }

  IVector3<TType> cross(const IVector3<TType> &other) const {
    return IVector3<TType>(y * other.z - z * other.y, z * other.x - x * other.z,
                           x * other.y - y * other.x);
  }
};