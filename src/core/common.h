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

inline double clamp(double x, double min, double max) {
  if (x < min) return min;
  if (x > max) return max;
  return x;
}

// The heart of our engine!
class Random {
 private:
  // Thread-local generator
  static thread_local std::mt19937 generator;

  // Global seed value (0 means use random seed)
  static uint32_t global_seed;

  // Thread-local initialization flag
  static thread_local bool initialized;

 public:
  // Set a specific seed for deterministic results (0 means use random seed)
  static void set_seed(uint32_t seed) {
    global_seed = seed;

    // If already initialized, re-seed the generator
    if (initialized) {
      generator.seed(seed != 0 ? seed : std::random_device{}());
    }
  }

  // Get the current seed
  static uint32_t get_seed() { return global_seed; }

  // Initialize or re-initialize the generator if needed
  static void initialize() {
    if (!initialized) {
      generator.seed(global_seed != 0 ? global_seed : std::random_device{}());
      initialized = true;
    }
  }

  // Returns a random double in range [0, 1)
  static double double_value() {
    initialize();
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    return distribution(generator);
  }

  // Returns a random double in range [min, max)
  static double double_value(double min, double max) { return min + (max - min) * double_value(); }

  // Returns a random integer in range [min, max]
  static int int_value(int min, int max) {
    initialize();
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generator);
  }
};

// Replace old functions with the new class methods for backward compatibility
inline double random_double() { return Random::double_value(); }

inline double random_double(double min, double max) { return Random::double_value(min, max); }

inline int random_int(int min, int max) { return Random::int_value(min, max); }
