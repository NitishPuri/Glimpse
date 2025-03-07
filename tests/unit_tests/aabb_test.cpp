#include "core/aabb.h"

#include "../test_cfg.h"

using namespace glimpse;

void aabb_test() {
  using namespace boost::ut;

  "aabb"_test = [] {
    // Test empty constructor
    aabb empty_box;
    expect(empty_box.x.min == glimpse::math::infinity);
    expect(empty_box.x.max == -glimpse::math::infinity);
    expect(empty_box.y.min == glimpse::math::infinity);
    expect(empty_box.y.max == -glimpse::math::infinity);
    expect(empty_box.z.min == glimpse::math::infinity);
    expect(empty_box.z.max == -glimpse::math::infinity);

    // Test two points constructor
    point3 p1(1.0, 2.0, 3.0);
    point3 p2(4.0, 5.0, 6.0);
    aabb box(p1, p2);
    expect(box.x.min == 1.0_d);
    expect(box.x.max == 4.0_d);
    expect(box.y.min == 2.0_d);
    expect(box.y.max == 5.0_d);
    expect(box.z.min == 3.0_d);
    expect(box.z.max == 6.0_d);

    "hit"_test = [] {
      point3 p1(1.0, 2.0, 3.0);
      point3 p2(4.0, 5.0, 6.0);
      aabb box(p1, p2);

      // Test hit detection
      {
        // Ray misses the box
        point3 origin(0, 0, 0);
        vec3 direction(1, 0, 0);
        ray r(origin, direction);
        interval ray_t(0.5, 10.0);
        expect(box.hit(r, ray_t) == false);

        // Ray hits the box
        origin = point3(0, 3, 4);
        direction = vec3(1, 0, 0);
        r = ray(origin, direction);
        expect(box.hit(r, ray_t) == true);
      }
    };

    // Test longest axis
    expect(box.longest_axis() == 2_i);  // All axes are equal length (3.0)

    // Test box addition with offset
    vec3 offset(1, 1, 1);
    aabb moved_box = box + offset;
    expect(moved_box.x.min == 2.0_d);
    expect(moved_box.x.max == 5.0_d);
    expect(moved_box.y.min == 3.0_d);
    expect(moved_box.y.max == 6.0_d);
    expect(moved_box.z.min == 4.0_d);
    expect(moved_box.z.max == 7.0_d);

    // Test intersection from inside the box
    "hit_from_inside"_test = [] {
      aabb box(point3(0, 0, 0), point3(2, 2, 2));
      point3 origin(1, 1, 1);  // Inside point
      vec3 direction(1, 0, 0);
      ray r(origin, direction);
      interval ray_t(0.5, 10.0);
      expect(box.hit(r, ray_t) == true);
    };

    // Test ray parallel to box faces
    "hit_parallel"_test = [] {
      aabb box(point3(0, 0, 0), point3(2, 2, 2));
      {
        ray r(point3(-1, 1, 1), vec3(1, 0, 0));
        interval ray_t(0.5, 10.0);
        expect(box.hit(r, ray_t) == true);
      }
      {
        ray r(point3(-1, 3, 1), vec3(1, 0, 0));
        interval ray_t(0.5, 10.0);
        expect(box.hit(r, ray_t) == false);
      }
    };

    // Test degenerate boxes (zero volume)
    "degenerate_box"_test = [] {
      aabb flat_box(point3(0, 0, 0), point3(2, 0, 2));  // Zero height
      ray r(point3(1, 1, 1), vec3(0, -1, 0));
      interval ray_t(0.5, 10.0);
      expect(flat_box.hit(r, ray_t) == true);
    };

    // Test ray interval constraints
    "ray_interval"_test = [] {
      aabb box(point3(0, 0, 0), point3(2, 2, 2));
      point3 origin(-2, 1, 1);
      vec3 direction(1, 0, 0);
      ray r(origin, direction);

      expect(box.hit(r, interval(0, 1)) == false);   // Too short
      expect(box.hit(r, interval(0, 2)) == false);   // Barely too short
      expect(box.hit(r, interval(0, 3)) == true);    // Just right
      expect(box.hit(r, interval(4, 10)) == false);  // Starts too late
    };

    // Test box combinations
    {
      aabb box1(point3(0, 0, 0), point3(2, 2, 2));
      aabb box2(point3(1, 1, 1), point3(3, 3, 3));
      aabb combined(box1, box2);
      expect(combined.x.min == 0.0_d);
      expect(combined.x.max == 3.0_d);
      expect(combined.y.min == 0.0_d);
      expect(combined.y.max == 3.0_d);
      expect(combined.z.min == 0.0_d);
      expect(combined.z.max == 3.0_d);
    }

    // Test axis_interval
    {
      expect(box.axis_interval(0).min == 1.0_d);
      expect(box.axis_interval(1).min == 2.0_d);
      expect(box.axis_interval(2).min == 3.0_d);
      expect(box.axis_interval(0).max == 4.0_d);
      expect(box.axis_interval(1).max == 5.0_d);
      expect(box.axis_interval(2).max == 6.0_d);
    }
  };
}