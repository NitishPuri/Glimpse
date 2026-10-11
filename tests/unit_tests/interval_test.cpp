#include "glimpse/util/interval.h"

#include "../test_cfg.h"

using namespace glimpse;

static ut::suite<"interval"> intervalSuite = [] {
  using namespace boost::ut;

  "interval"_test = [] {
    // Test default constructor (empty interval)
    "default_constructor"_test = [] {
      Interval i;
      expect(i.min == glimpse::math::infinity);
      expect(i.max == -glimpse::math::infinity);
    };

    // Test min/max constructor
    "min_max_constructor"_test = [] {
      Interval i(2.0, 5.0);
      expect(i.min == 2.0_d);
      expect(i.max == 5.0_d);
    };

    // Test interval-interval constructor
    "interval_interval_constructor"_test = [] {
      Interval i1(2.0, 5.0);
      Interval i2(1.0, 6.0);
      Interval combined(i1, i2);
      expect(combined.min == 1.0_d);
      expect(combined.max == 6.0_d);

      // Test when first interval has larger values
      Interval i3(7.0, 10.0);
      Interval combined2(i3, i1);
      expect(combined2.min == 2.0_d);
      expect(combined2.max == 10.0_d);
    };

    // Test size() method
    "size"_test = [] {
      Interval i(2.0, 5.0);
      expect(i.size() == 3.0_d);

      Interval empty;
      expect(empty.size() == -glimpse::math::infinity);
    };

    // Test contains() method
    "contains"_test = [] {
      Interval i(2.0, 5.0);
      expect(i.contains(2.0) == true);
      expect(i.contains(3.5) == true);
      expect(i.contains(5.0) == true);
      expect(i.contains(1.0) == false);
      expect(i.contains(6.0) == false);
    };

    // Test surrounds() method
    "surrounds"_test = [] {
      Interval i(2.0, 5.0);
      expect(i.surrounds(2.0) == false);  // Boundary value
      expect(i.surrounds(3.5) == true);
      expect(i.surrounds(5.0) == false);  // Boundary value
      expect(i.surrounds(1.0) == false);
      expect(i.surrounds(6.0) == false);
    };

    // Test clamp() method
    "clamp"_test = [] {
      Interval i(2.0, 5.0);
      expect(i.clamp(3.0) == 3.0_d);
      expect(i.clamp(1.0) == 2.0_d);
      expect(i.clamp(6.0) == 5.0_d);
      expect(i.clamp(2.0) == 2.0_d);
      expect(i.clamp(5.0) == 5.0_d);
    };

    // Test expand() method
    "expand"_test = [] {
      Interval i(2.0, 5.0);
      auto expanded = i.expand(2.0);
      expect(expanded.min == 1.0_d);
      expect(expanded.max == 6.0_d);
    };

    // Test operators
    "operators"_test = [] {
      // Test operator+
      Interval i(2.0, 5.0);
      auto shifted = i + 3.0;
      expect(shifted.min == 5.0_d);
      expect(shifted.max == 8.0_d);

      // Test reverse operator+
      auto shifted2 = 3.0 + i;
      expect(shifted2.min == 5.0_d);
      expect(shifted2.max == 8.0_d);
    };

    // Test with edge cases
    "edge_cases"_test = [] {
      // Zero-size interval
      Interval zeroSize(3.0, 3.0);
      expect(zeroSize.size() == 0.0_d);
      expect(zeroSize.contains(3.0) == true);
      expect(zeroSize.surrounds(3.0) == false);

      // Negative size interval (invalid)
      Interval negativeSize(5.0, 2.0);
      expect(negativeSize.size() == -3.0_d);
    };
  };
};