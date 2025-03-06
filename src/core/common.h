#pragma once

#include <chrono>
#include <iostream>
#include <limits>
#include <memory>
#include <random>

// Using
// using chrono = std::chrono;
using std::make_shared;
using std::shared_ptr;
using std::sqrt;

// Constants
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility functions
inline double degrees_to_radians(double degrees) { return degrees * pi / 180.0; }

// Returns a random number in range [0, 1)
// inline double random_double() { return rand() / (RAND_MAX + 1.0); }
inline double random_double() {
  static thread_local std::mt19937 generator;
  std::uniform_real_distribution<double> distribution(0.0, 1.0);
  return distribution(generator);
}

inline double random_double(double min, double max) { return min + (max - min) * random_double(); }

inline int random_int(int min, int max) {
  // Returns a random integer in [min,max].
  return static_cast<int>(random_double(min, max + 1));
}

inline double clamp(double x, double min, double max) {
  if (x < min) return min;
  if (x > max) return max;
  return x;
}
