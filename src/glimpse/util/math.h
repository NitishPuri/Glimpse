#pragma once

#include <cmath>
#include <limits>
#include <memory>

namespace glimpse {

// Using
using std::make_shared;
using std::shared_ptr;
using std::sqrt;

namespace math {
// Constants
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility functions
inline double degrees_to_radians(double degrees) { return degrees * pi / 180.0; }

inline double clamp(double x, double min, double max) {
  if (x < min) return min;
  if (x > max) return max;
  return x;
}

}  // namespace math

}  // namespace glimpse
