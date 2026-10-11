#pragma once

#include <cstdint>
#include <random>

namespace glimpse {

// The heart of our engine!
class Random {
 private:
  // Thread-local generator
  static inline thread_local std::mt19937 generator;

  // Global seed value (0 means use random seed)
  static inline uint32_t global_seed = 0;

  // Thread-local initialization flag
  static inline thread_local bool initialized = false;

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

}  // namespace glimpse
