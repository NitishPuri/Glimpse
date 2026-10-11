
#include "glimpse/film.h"

#include "../test_cfg.h"

using namespace glimpse;

static ut::suite<"film"> filmSuite = [] {
  using namespace boost::ut;

  "film.construction"_test = [] {
    {
      Film f;
      expect(f.getSampleCount(0, 0) == 0_i);
    }
    {
      Film f(10, 20);
      expect(f.getSampleCount(0, 0) == 0_i);
      expect(f.getSampleCount(9, 19) == 0_i);
    }
  };

  "film.initialization"_test = [] {
    Film f;
    f.initialize(5, 5);
    expect(f.getSampleCount(0, 0) == 0_i);
    expect(f.getSampleCount(4, 4) == 0_i);
    expect(f.getAverageSampleCount() == 0_i);
  };

  "film.sample_management"_test = [] {
    Film f(2, 2);

    f.addSample(0, 0, Vec3(1.0f, 1.0f, 1.0f));
    expect(f.getSampleCount(0, 0) == 1_i);
    expect(f.getAverageSampleCount() == 0_i);  // 1/4 rounded down

    f.addSample(0, 0, Vec3(2.0f, 2.0f, 2.0f));
    expect(f.getSampleCount(0, 0) == 2_i);

    expect(f.getSampleCount(1, 1) == 0_i);
  };

  "film.sample_retrieval"_test = [] {
    Film f(1, 1);

    f.addSample(0, 0, Vec3(1.0f, 2.0f, 3.0f));
    f.addSample(0, 0, Vec3(3.0f, 4.0f, 5.0f));

    auto accum = f.getAccumulatedSample(0, 0);
    expect(accum.x() == 4.0_f);
    expect(accum.y() == 6.0_f);
    expect(accum.z() == 8.0_f);

    auto sample = f.getSample(0, 0);
    expect(sample.x() == 2.0_f);
    expect(sample.y() == 3.0_f);
    expect(sample.z() == 4.0_f);
  };

  "film.edge_cases"_test = [] {
    Film f(2, 2);

    // Zero samples case
    auto zeroSample = f.getSample(0, 0);
    expect(zeroSample.x() == 0.0_f);
    expect(zeroSample.y() == 0.0_f);
    expect(zeroSample.z() == 0.0_f);

    // Variance with insufficient samples
    auto zeroVariance = f.getVariance(0, 0);
    expect(zeroVariance.x() == 0.0_f);
    expect(zeroVariance.y() == 0.0_f);
    expect(zeroVariance.z() == 0.0_f);

    // Boundary coordinates
    f.addSample(1, 1, Vec3(1.0f, 1.0f, 1.0f));
    expect(f.getSampleCount(1, 1) == 1_i);
  };

  // linearRGB() is the running mean (linear radiance) as a FloatImage. Film row j counts up from the bottom like
  // camera v, FloatImage row 0 is the top, so Film (x, j) lands in FloatImage row height-1-j.
  "film.linearRGB"_test = [] {
    Film f(2, 3);
    f.addSample(1, 0, Vec3(2.0, 4.0, 6.0));  // bottom row
    f.addSample(1, 0, Vec3(4.0, 8.0, 12.0));  // mean (3, 6, 9): above 1, not clamped
    f.addSample(0, 2, Vec3(0.25, 0.5, 0.75));  // top row
    const FloatImage img = f.linearRGB();
    expect(img.width == 2_i && img.height == 3_i);
    const size_t bottom = img.index(1, 2), top = img.index(0, 0);
    expect(img.rgb[bottom] == 3.0_f && img.rgb[bottom + 1] == 6.0_f && img.rgb[bottom + 2] == 9.0_f);
    expect(img.rgb[top] == 0.25_f && img.rgb[top + 2] == 0.75_f);
    expect(img.rgb[img.index(0, 1)] == 0.0_f) << "pixels without samples are black";
  };
};
