
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
};