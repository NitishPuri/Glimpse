#include "glimpse/ray.h"

#include "../test_cfg.h"

using namespace glimpse;

static ut::suite<"ray"> raySuite = [] {
  using namespace boost::ut;

  "ray"_test = [] {
    "default_construction"_test = [] {
      Ray r;
      expect(r.origin() == Vec3(0, 0, 0));
      expect(r.direction() == Vec3(0, 0, 0));
      expect(r.time() == 0.0_d);
    };

    "construction_with_origin_direction"_test = [] {
      Point3 orig(1, 2, 3);
      Vec3 dir(4, 5, 6);
      Ray r(orig, dir);
      expect(r.origin() == orig);
      expect(r.direction() == dir);
      expect(r.time() == 0.0_d);
    };

    "construction_with_time"_test = [] {
      Point3 orig(1, 2, 3);
      Vec3 dir(4, 5, 6);
      double time = 1.5;
      Ray r(orig, dir, time);
      expect(r.origin() == orig);
      expect(r.direction() == dir);
      expect(r.time() == time);
    };

    "point_at_parameter"_test = [] {
      Point3 orig(1, 0, 0);
      Vec3 dir(1, 0, 0);
      Ray r(orig, dir);

      auto pt = r.at(2.0);
      expect(pt.x() == 3.0_d);
      expect(pt.y() == 0.0_d);
      expect(pt.z() == 0.0_d);
    };
  };
};