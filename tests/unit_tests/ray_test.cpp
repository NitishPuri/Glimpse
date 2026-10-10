#include "core/ray.h"

#include "../test_cfg.h"

using namespace glimpse;

void ray_test() {
  using namespace boost::ut;

  "ray"_test = [] {
    "default_construction"_test = [] {
      ray r;
      expect(r.origin() == vec3(0, 0, 0));
      expect(r.direction() == vec3(0, 0, 0));
      expect(r.time() == 0.0_d);
    };

    "construction_with_origin_direction"_test = [] {
      point3 orig(1, 2, 3);
      vec3 dir(4, 5, 6);
      ray r(orig, dir);
      expect(r.origin() == orig);
      expect(r.direction() == dir);
      expect(r.time() == 0.0_d);
    };

    "construction_with_time"_test = [] {
      point3 orig(1, 2, 3);
      vec3 dir(4, 5, 6);
      double time = 1.5;
      ray r(orig, dir, time);
      expect(r.origin() == orig);
      expect(r.direction() == dir);
      expect(r.time() == time);
    };

    "point_at_parameter"_test = [] {
      point3 orig(1, 0, 0);
      vec3 dir(1, 0, 0);
      ray r(orig, dir);

      auto pt = r.at(2.0);
      expect(pt.x() == 3.0_d);
      expect(pt.y() == 0.0_d);
      expect(pt.z() == 0.0_d);
    };
  };
}