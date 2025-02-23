#pragma once

#include <cmath>
#include <cstdlib>
#include <limits>
#include <memory>

// Using
using std::make_shared;
using std::shared_ptr;
using std::sqrt;

// Constants
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility functions
inline double degrees_to_radians(double degrees) {
  return degrees * pi / 180.0;
}

// Returns a random number in range [0, 1)
inline double random_double() { return rand() / (RAND_MAX + 1.0); }

inline double random_double(double min, double max) {
  return min + (max - min) * random_double();
}

inline int random_int(int min, int max) {
  // Returns a random integer in [min,max].
  return static_cast<int>(random_double(min, max + 1));
}

inline double clamp(double x, double min, double max) {
  if (x < min) return min;
  if (x > max) return max;
  return x;
}

// #include "logger.h"
// const Logger& getLogger(const std::string& log_file_path = "./log.txt") {
//   static Logger logger(log_file_path);
//   return logger;
// }

// Common headers
// clang-format off
#include "vec3.h"
#include "ray.h"
#include "bvh_node.h"
#include "camera.h"
#include "hittable.h"
#include "material.h"
#include "scenes.h"
#include "texture.h"
// clang-format on
