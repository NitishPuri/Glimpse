#pragma once

// Math constants (pi, infinity), degreesToRadians and clamp, plus the std aliases the core uses (shared_ptr, sqrt).

#include <cmath>
#include <limits>
#include <memory>
#include <numbers>

namespace glimpse {

// Using
using std::make_shared;
using std::shared_ptr;
using std::sqrt;

namespace math {
// Constants
const double infinity = std::numeric_limits<double>::infinity();
const double pi = std::numbers::pi;

// Utility functions
inline double degreesToRadians(double degrees) { return degrees * pi / 180.0; }

inline double clamp(double x, double min, double max) {
  if (x < min) return min;
  if (x > max) return max;
  return x;
}

}  // namespace math

}  // namespace glimpse
