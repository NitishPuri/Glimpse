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
  static inline uint32_t globalSeed = 0;

  // Thread-local initialization flag
  static inline thread_local bool initialized = false;

 public:
  // Set a specific seed for deterministic results (0 means use random seed)
  static void setSeed(uint32_t seed) {
    globalSeed = seed;

    // If already initialized, re-seed the generator
    if (initialized) {
      generator.seed(seed != 0 ? seed : std::random_device{}());
    }
  }

  // Get the current seed
  static uint32_t getSeed() { return globalSeed; }

  // Initialize or re-initialize the generator if needed
  static void initialize() {
    if (!initialized) {
      generator.seed(globalSeed != 0 ? globalSeed : std::random_device{}());
      initialized = true;
    }
  }

  // Returns a random double in range [0, 1)
  static double doubleValue() {
    initialize();
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    return distribution(generator);
  }

  // Returns a random double in range [min, max)
  static double doubleValue(double min, double max) { return min + (max - min) * doubleValue(); }

  // Returns a random integer in range [min, max]
  static int intValue(int min, int max) {
    initialize();
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generator);
  }
};

// Replace old functions with the new class methods for backward compatibility
inline double randomDouble() { return Random::doubleValue(); }

inline double randomDouble(double min, double max) { return Random::doubleValue(min, max); }

inline int randomInt(int min, int max) { return Random::intValue(min, max); }

}  // namespace glimpse
