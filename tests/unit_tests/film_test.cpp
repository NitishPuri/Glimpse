
#include "core/film.h"

#include "../test_cfg.h"

using namespace glimpse;

void film_test() {
  using namespace boost::ut;

  "film.construction"_test = [] {
    {
      Film f;
      expect(f.get_sample_count(0, 0) == 0_i);
    }
    {
      Film f(10, 20);
      expect(f.get_sample_count(0, 0) == 0_i);
      expect(f.get_sample_count(9, 19) == 0_i);
    }
  };

  "film.initialization"_test = [] {
    Film f;
    f.initialize(5, 5);
    expect(f.get_sample_count(0, 0) == 0_i);
    expect(f.get_sample_count(4, 4) == 0_i);
    expect(f.get_average_sample_count() == 0_i);
  };

  "film.sample_management"_test = [] {
    Film f(2, 2);

    f.add_sample(0, 0, vec3(1.0f, 1.0f, 1.0f));
    expect(f.get_sample_count(0, 0) == 1_i);
    expect(f.get_average_sample_count() == 0_i);  // 1/4 rounded down

    f.add_sample(0, 0, vec3(2.0f, 2.0f, 2.0f));
    expect(f.get_sample_count(0, 0) == 2_i);

    expect(f.get_sample_count(1, 1) == 0_i);
  };

  "film.sample_retrieval"_test = [] {
    Film f(1, 1);

    f.add_sample(0, 0, vec3(1.0f, 2.0f, 3.0f));
    f.add_sample(0, 0, vec3(3.0f, 4.0f, 5.0f));

    auto accum = f.get_accumulated_sample(0, 0);
    expect(accum.x() == 4.0_f);
    expect(accum.y() == 6.0_f);
    expect(accum.z() == 8.0_f);

    auto sample = f.get_sample(0, 0);
    expect(sample.x() == 2.0_f);
    expect(sample.y() == 3.0_f);
    expect(sample.z() == 4.0_f);
  };

  "film.edge_cases"_test = [] {
    Film f(2, 2);

    // Zero samples case
    auto zero_sample = f.get_sample(0, 0);
    expect(zero_sample.x() == 0.0_f);
    expect(zero_sample.y() == 0.0_f);
    expect(zero_sample.z() == 0.0_f);

    // Variance with insufficient samples
    auto zero_variance = f.get_variance(0, 0);
    expect(zero_variance.x() == 0.0_f);
    expect(zero_variance.y() == 0.0_f);
    expect(zero_variance.z() == 0.0_f);

    // Boundary coordinates
    f.add_sample(1, 1, vec3(1.0f, 1.0f, 1.0f));
    expect(f.get_sample_count(1, 1) == 1_i);
  };
}