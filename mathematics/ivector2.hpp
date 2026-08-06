#pragma once

#include <cmath>

template <typename TType> struct IVector2 {
  TType x;
  TType y;

  IVector2<TType>(TType x, TType y) : x(x), y(y) {}
  IVector2<TType>() : x(), y() {}
  ~IVector2<TType>() {}

  // Pass by const reference (&) to avoid copying!
  IVector2<TType> operator+(const IVector2<TType> &other) const {
    return IVector2<TType>(x + other.x, y + other.y);
  }
  IVector2<TType> operator-(const IVector2<TType> &other) const {
    return IVector2<TType>(x - other.x, y - other.y);
  }
  IVector2<TType> operator*(const IVector2<TType> &other) const {
    return IVector2<TType>(x * other.x, y * other.y);
  }
  IVector2<TType> operator/(const IVector2<TType> &other) const {
    return IVector2<TType>(x / other.x, y / other.y);
  }
  bool operator==(const IVector2<TType> &other) const {
    return x == other.x && y == other.y;
  }
  bool operator!=(const IVector2<TType> &other) const {
    return x != other.x || y != other.y;
  }

  float length() const { return std::sqrt(x * x + y * y); }

  IVector2<float> normalize() const {
    // Strictly use float here so we don't lose precision
    float len = length();
    if (len == 0.0f)
      return IVector2<float>(0.0f, 0.0f);

    // Cast x and y to floats before dividing to ensure precise decimal math
    return IVector2<float>(static_cast<float>(x) / len,
                           static_cast<float>(y) / len);
  }

  float dot(const IVector2<TType> &other) const {
    return static_cast<float>(x * other.x + y * other.y);
  }

  // this is just returning the normal to the vector
  IVector2<TType> cross() const { return IVector2<TType>(-y, x); }
};