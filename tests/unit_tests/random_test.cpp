#include <thread>

#include "glimpse/util/rng.h"

//
#include "../test_cfg.h"
using namespace glimpse;

static ut::suite<"random"> randomSuite = [] {
  using namespace boost::ut;

  "random_seed"_test = [] {
    // Setting the same seed should produce identical sequences
    Random::setSeed(1234);
    auto a1 = randomDouble();
    auto a2 = randomDouble();

    Random::setSeed(1234);
    auto b1 = randomDouble();
    auto b2 = randomDouble();

    expect(a1 == b1);
    expect(a2 == b2);

    // Setting different seeds should produce different sequences
    Random::setSeed(42);
    auto c1 = randomDouble();
    auto c2 = randomDouble();

    expect(!(a1 == c1 && a2 == c2));
  };

  "random_range"_test = [] {
    Random::setSeed(1234);

    // Test doubleValue() is in [0,1)
    for (int i = 0; i < 100; i++) {
      auto val = Random::doubleValue();
      expect(val >= 0.0);
      expect(val < 1.0);
    }

    // Test doubleValue(min,max) is in [min,max)
    double min = -5.0, max = 10.0;
    for (int i = 0; i < 100; i++) {
      auto val = Random::doubleValue(min, max);
      expect(val >= min);
      expect(val < max);
    }

    // Test intValue(min,max) is in [min,max]
    int imin = -10, imax = 20;
    for (int i = 0; i < 100; i++) {
      auto val = Random::intValue(imin, imax);
      expect(val >= imin);
      expect(val <= imax);
    }
  };

  "random_thread_local"_test = [] {
    // Test that different threads get different sequences with same seed
    std::vector<double> thread1Values;
    std::vector<double> thread2Values;

    std::thread t1([&thread1Values]() {
      Random::setSeed(99);
      for (int i = 0; i < 5; i++) {
        thread1Values.push_back(Random::doubleValue());
      }
    });

    std::thread t2([&thread2Values]() {
      Random::setSeed(99);
      for (int i = 0; i < 5; i++) {
        thread2Values.push_back(Random::doubleValue());
      }
    });

    t1.join();
    t2.join();

    // Each thread should get the same sequence independently
    expect(thread1Values == thread2Values);
  };

  "random_default_seed"_test = [] {
    // Test that seed 0 gives different sequences each time
    Random::setSeed(0);
    auto seq1First = Random::doubleValue();
    [[maybe_unused]]
    auto seq1Second = Random::doubleValue();

    Random::setSeed(0);
    auto seq2First = Random::doubleValue();

    // Can't guarantee they'll be different every time, but they should be *usually*
    // This is a probabilistic test, so it might occasionally fail
    expect(seq1First != seq2First) << "Using seed 0 should give different sequences";
  };
};