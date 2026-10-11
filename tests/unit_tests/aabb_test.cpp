#include "glimpse/util/aabb.h"

#include "../test_cfg.h"

using namespace glimpse;

static ut::suite<"aabb"> aabbSuite = [] {
  using namespace boost::ut;

  "aabb"_test = [] {
    // Test empty constructor
    AABB emptyBox;
    expect(emptyBox.x.min == glimpse::math::infinity);
    expect(emptyBox.x.max == -glimpse::math::infinity);
    expect(emptyBox.y.min == glimpse::math::infinity);
    expect(emptyBox.y.max == -glimpse::math::infinity);
    expect(emptyBox.z.min == glimpse::math::infinity);
    expect(emptyBox.z.max == -glimpse::math::infinity);

    // Test two points constructor
    Point3 p1(1.0, 2.0, 3.0);
    Point3 p2(4.0, 5.0, 6.0);
    AABB box(p1, p2);
    expect(box.x.min == 1.0_d);
    expect(box.x.max == 4.0_d);
    expect(box.y.min == 2.0_d);
    expect(box.y.max == 5.0_d);
    expect(box.z.min == 3.0_d);
    expect(box.z.max == 6.0_d);

    "hit"_test = [] {
      Point3 p1(1.0, 2.0, 3.0);
      Point3 p2(4.0, 5.0, 6.0);
      AABB box(p1, p2);

      // Test hit detection
      {
        // Ray misses the box
        Point3 origin(0, 0, 0);
        Vec3 direction(1, 0, 0);
        Ray r(origin, direction);
        Interval rayT(0.5, 10.0);
        expect(box.hit(r, rayT) == false);

        // Ray hits the box
        origin = Point3(0, 3, 4);
        direction = Vec3(1, 0, 0);
        r = Ray(origin, direction);
        expect(box.hit(r, rayT) == true);
      }
    };

    // Test longest axis
    expect(box.longestAxis() == 2_i);  // All axes are equal length (3.0)

    // Test box addition with offset
    Vec3 offset(1, 1, 1);
    AABB movedBox = box + offset;
    expect(movedBox.x.min == 2.0_d);
    expect(movedBox.x.max == 5.0_d);
    expect(movedBox.y.min == 3.0_d);
    expect(movedBox.y.max == 6.0_d);
    expect(movedBox.z.min == 4.0_d);
    expect(movedBox.z.max == 7.0_d);

    // Test intersection from inside the box
    "hit_from_inside"_test = [] {
      AABB box(Point3(0, 0, 0), Point3(2, 2, 2));
      Point3 origin(1, 1, 1);  // Inside point
      Vec3 direction(1, 0, 0);
      Ray r(origin, direction);
      Interval rayT(0.5, 10.0);
      expect(box.hit(r, rayT) == true);
    };

    // Test ray parallel to box faces
    "hit_parallel"_test = [] {
      AABB box(Point3(0, 0, 0), Point3(2, 2, 2));
      {
        Ray r(Point3(-1, 1, 1), Vec3(1, 0, 0));
        Interval rayT(0.5, 10.0);
        expect(box.hit(r, rayT) == true);
      }
      {
        Ray r(Point3(-1, 3, 1), Vec3(1, 0, 0));
        Interval rayT(0.5, 10.0);
        expect(box.hit(r, rayT) == false);
      }
    };

    // Test degenerate boxes (zero volume)
    "degenerate_box"_test = [] {
      AABB flatBox(Point3(0, 0, 0), Point3(2, 0, 2));  // Zero height
      Ray r(Point3(1, 1, 1), Vec3(0, -1, 0));
      Interval rayT(0.5, 10.0);
      expect(flatBox.hit(r, rayT) == true);
    };

    // Test ray interval constraints
    "ray_interval"_test = [] {
      AABB box(Point3(0, 0, 0), Point3(2, 2, 2));
      Point3 origin(-2, 1, 1);
      Vec3 direction(1, 0, 0);
      Ray r(origin, direction);

      expect(box.hit(r, Interval(0, 1)) == false);   // Too short
      expect(box.hit(r, Interval(0, 2)) == false);   // Barely too short
      expect(box.hit(r, Interval(0, 3)) == true);    // Just right
      expect(box.hit(r, Interval(4, 10)) == false);  // Starts too late
    };

    // Test box combinations
    {
      AABB box1(Point3(0, 0, 0), Point3(2, 2, 2));
      AABB box2(Point3(1, 1, 1), Point3(3, 3, 3));
      AABB combined(box1, box2);
      expect(combined.x.min == 0.0_d);
      expect(combined.x.max == 3.0_d);
      expect(combined.y.min == 0.0_d);
      expect(combined.y.max == 3.0_d);
      expect(combined.z.min == 0.0_d);
      expect(combined.z.max == 3.0_d);
    }

    // Test axisInterval
    {
      expect(box.axisInterval(0).min == 1.0_d);
      expect(box.axisInterval(1).min == 2.0_d);
      expect(box.axisInterval(2).min == 3.0_d);
      expect(box.axisInterval(0).max == 4.0_d);
      expect(box.axisInterval(1).max == 5.0_d);
      expect(box.axisInterval(2).max == 6.0_d);
    }
  };
};