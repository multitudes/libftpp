#pragma once

template <typename TType> struct Vector {
  TType x;
  TType y;

  Vector<TType>(TType x, TType y) : x(x), y(y) {}
  Vector<TType>() : x(), y() {}
  ~Vector<TType>() {}

  // +,-, *, /, ==, !=.
  Vector<TType> operator+(const Vector<TType> other) {
    return Vector<TType>(x + other.x, y + other.y);
  }
  Vector<TType> operator-(const Vector<TType> other) {
    return Vector<TType>(x - other.x, y - other.y);
  }
  TType operator*(const Vector<TType> other) {
    return Vector<TType>(x * other.x, y * other.y);
  }
  TType operator/(const Vector<TType> other) {
    return Vector<TType>(x / other.x, y / other.y);
  }
  bool operator==(const Vector<TType> other) {
    return x == other.x && y == other.y;
  }
  bool operator!=(const Vector<TType> other) {
    return x != other.x || y != other.y;
  }

  // additional methods
  // Expected: Length of vec1: 5 (or sqrt(3*3 + 4*4))
  TType length() {};

  // Expected: Normalized vec1 = (0.6, 0.8)
  Vector<TType> normalize() {};

  // Expected: Dot product of vec1 and vec2: 11 (or 3*1 + 4*2)
  TType dot(Vector<TType> other) { return x * other.x + y * other.y; };

  // probably they mean the normal
  Vector<TType> cross() { return Vector<TType>(-x, y); };
};