#include <thread>

#include "core/common.h"

//
#include "../test_cfg.h"
using namespace glimpse;

void random_test() {
  using namespace boost::ut;

  "random_seed"_test = [] {
    // Setting the same seed should produce identical sequences
    Random::set_seed(1234);
    auto a1 = random_double();
    auto a2 = random_double();

    Random::set_seed(1234);
    auto b1 = random_double();
    auto b2 = random_double();

    expect(a1 == b1);
    expect(a2 == b2);

    // Setting different seeds should produce different sequences
    Random::set_seed(42);
    auto c1 = random_double();
    auto c2 = random_double();

    expect(!(a1 == c1 && a2 == c2));
  };

  "random_range"_test = [] {
    Random::set_seed(1234);

    // Test double_value() is in [0,1)
    for (int i = 0; i < 100; i++) {
      auto val = Random::double_value();
      expect(val >= 0.0);
      expect(val < 1.0);
    }

    // Test double_value(min,max) is in [min,max)
    double min = -5.0, max = 10.0;
    for (int i = 0; i < 100; i++) {
      auto val = Random::double_value(min, max);
      expect(val >= min);
      expect(val < max);
    }

    // Test int_value(min,max) is in [min,max]
    int imin = -10, imax = 20;
    for (int i = 0; i < 100; i++) {
      auto val = Random::int_value(imin, imax);
      expect(val >= imin);
      expect(val <= imax);
    }
  };

  "random_thread_local"_test = [] {
    // Test that different threads get different sequences with same seed
    std::vector<double> thread1_values;
    std::vector<double> thread2_values;

    std::thread t1([&thread1_values]() {
      Random::set_seed(99);
      for (int i = 0; i < 5; i++) {
        thread1_values.push_back(Random::double_value());
      }
    });

    std::thread t2([&thread2_values]() {
      Random::set_seed(99);
      for (int i = 0; i < 5; i++) {
        thread2_values.push_back(Random::double_value());
      }
    });

    t1.join();
    t2.join();

    // Each thread should get the same sequence independently
    expect(thread1_values == thread2_values);
  };

  "random_default_seed"_test = [] {
    // Test that seed 0 gives different sequences each time
    Random::set_seed(0);
    auto seq1_first = Random::double_value();
    [[maybe_unused]]
    auto seq1_second = Random::double_value();

    Random::set_seed(0);
    auto seq2_first = Random::double_value();

    // Can't guarantee they'll be different every time, but they should be *usually*
    // This is a probabilistic test, so it might occasionally fail
    expect(seq1_first != seq2_first) << "Using seed 0 should give different sequences";
  };
}