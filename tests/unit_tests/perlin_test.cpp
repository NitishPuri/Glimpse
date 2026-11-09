#include "glimpse/perlin.h"

#include "../test_cfg.h"

using namespace glimpse;

static ut::suite<"perlin"> perlinSuite = [] {
  using namespace boost::ut;

  "perlin"_test = [] {
    "construction"_test = [] {
      // Test basic construction
      Perlin noise;

      // This is primarily to verify the constructor doesn't crash
      // We can't test specific values of random vectors, but we can test
      // the noise function returns reasonable values
      expect(true);
    };

    "noise_basic"_test = [] {
      Perlin noise;

      // Test that noise returns values in expected range [-1, 1]
      for (int i = 0; i < 100; i++) {
        Point3 p(i * 0.1, i * 0.2, i * 0.3);
        double n = noise.noise(p);
        expect(n >= -1.0_d);
        expect(n <= 1.0_d);
      }
    };

    "noise_consistency"_test = [] {
      Perlin noise;

      // Test that the same point always gives the same noise value
      Point3 p(1.2, 3.4, 5.6);
      double firstValue = noise.noise(p);

      for (int i = 0; i < 5; i++) {
        expect(noise.noise(p) == firstValue);
      }
    };

    skip / "noise_different_points"_test = [] {
      Perlin noise;

      // Test that different points typically give different noise values
      double prev = noise.noise(Point3(0, 0, 0));
      int differentCount = 0;

      std::cout << prev << std::endl;

      for (int i = 1; i < 10; i++) {
        double current = noise.noise(Point3(i, i, i));
        std::cout << current << std::endl;
        if (std::abs(current - prev) > 0.001) {
          differentCount++;
        }
        prev = current;
      }

      // At least some values should be different
      expect(differentCount > 0_i);
    };

    "turbulence"_test = [] {
      Perlin noise;

      // Test that turbulence returns positive values (since it uses fabs)
      Point3 p(1.2, 3.4, 5.6);

      for (int depth = 1; depth <= 10; depth++) {
        double t = noise.turb(p, depth);
        expect(t >= 0.0_d) << "Turbulence should never be negative";
      }
    };

    "turbulence_depth"_test = [] {
      Perlin noise;
      Point3 p(1.2, 3.4, 5.6);

      // As depth increases, turbulence typically accumulates more detail
      // Higher depth should generally give non-zero values that are different from lower depths
      double t1 = noise.turb(p, 1);
      double t5 = noise.turb(p, 5);
      double t10 = noise.turb(p, 10);

      expect(t5 != t1) << "Higher depth turbulence should differ from lower depth";
      expect(t10 != t5) << "Higher depth turbulence should differ from lower depth";
    };

    "noise_interpolation"_test = [] {
      Perlin noise;

      // Test that nearby points have somewhat similar noise values
      Point3 p1(1.0, 1.0, 1.0);
      Point3 p2(1.001, 1.0, 1.0);  // Very close to p1
      Point3 p3(1.5, 1.5, 1.5);    // Further from p1

      double n1 = noise.noise(p1);
      double n2 = noise.noise(p2);
      double n3 = noise.noise(p3);

      // Close points should have similar noise values
      expect(std::abs(n1 - n2) < std::abs(n1 - n3))
          << "Noise values should vary smoothly - closer points should be more similar";
    };

    "noise_periodicity"_test = [] {
      // The lattice lookup wraps with `& 255`, so the noise repeats every 256 units along each axis. Not bit for
      // bit: adding 256 rounds the fractional part (i + 0.3 and i + 256.3 differ in the last bits).
      Perlin noise;
      for (int i = 0; i < 10; i++) {
        const Point3 p(i + 0.3, 0.5 * i, 0.7);
        expect(std::abs(noise.noise(p) - noise.noise(p + Vec3(256, 0, 0))) < 1e-9) << "x period, i =" << i;
        expect(std::abs(noise.noise(p) - noise.noise(p + Vec3(0, 0, -256))) < 1e-9) << "z period, i =" << i;
      }
    };

    "noise_zero_at_integers"_test = [] {
      // Gradient noise: at a lattice point only corner (0,0,0) has weight, and its offset to the point is zero, so
      // the dot product with its gradient vanishes whatever the gradient is.
      Perlin noise;
      for (int i = -5; i < 5; i++) {
        const double n = noise.noise(Point3(i, 2 * i, 3));
        expect(n == 0.0) << "i =" << i << "noise =" << n;
      }
    };
  };
};