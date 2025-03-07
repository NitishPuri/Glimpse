#include "core/hittables/sphere.h"

#include "core/material.h"

//
#include "../test_cfg.h"

using namespace glimpse;

void sphere_test() {
  using namespace boost::ut;

  "sphere"_test = [] {
    "construction"_test = [] {
      // Test stationary sphere construction
      {
        point3 center(1, 2, 3);
        double radius = 2.0;
        auto mat = make_shared<lambertian>(color(0.5, 0.5, 0.5));
        sphere s(center, radius, mat);

        // Test bounding box
        auto bbox = s.bounding_box();
        expect(bbox.x.min == -1.0_d);
        expect(bbox.x.max == 3.0_d);
        expect(bbox.y.min == 0.0_d);
        expect(bbox.y.max == 4.0_d);
        expect(bbox.z.min == 1.0_d);
        expect(bbox.z.max == 5.0_d);
      }

      // Test moving sphere construction
      {
        point3 center1(1, 2, 3);
        point3 center2(4, 5, 6);
        double radius = 2.0;
        auto mat = make_shared<lambertian>(color(0.5, 0.5, 0.5));
        sphere s(center1, center2, radius, mat);

        // Test bounding box encompasses both positions
        auto bbox = s.bounding_box();
        expect(bbox.x.min == -1.0_d);
        expect(bbox.x.max == 6.0_d);
        expect(bbox.y.min == 0.0_d);
        expect(bbox.y.max == 7.0_d);
        expect(bbox.z.min == 1.0_d);
        expect(bbox.z.max == 8.0_d);
      }

      // Test zero/negative radius
      {
        point3 center(1, 2, 3);
        auto mat = make_shared<lambertian>(color(0.5, 0.5, 0.5));

        sphere s_zero(center, 0, mat);
        sphere s_negative(center, -1, mat);

        // Both should have radius = 0 due to std::fmax(0, radius)
        auto bbox_zero = s_zero.bounding_box();
        expect(bbox_zero.x.min == 1.0_d);
        expect(bbox_zero.x.max == 1.0_d);

        auto bbox_negative = s_negative.bounding_box();
        expect(bbox_negative.x.min == 1.0_d);
        expect(bbox_negative.x.max == 1.0_d);
      }
    };

    "hit_detection"_test = [] {
      // Test stationary sphere hit detection
      {
        point3 center(0, 0, 0);
        double radius = 1.0;
        auto mat = make_shared<lambertian>(color(0.5, 0.5, 0.5));
        sphere s(center, radius, mat);

        // Ray hits sphere directly
        ray r(point3(0, 0, -2), vec3(0, 0, 1));
        hit_record rec;
        expect(s.hit(r, interval(0.001, glimpse::math::infinity), rec) == true);
        expect(rec.t == 1.0_d);
        expect(rec.p.z() == -1.0_d);
        expect(rec.normal.z() == -1.0_d);
        expect(rec.front_face == true);

        // Ray misses sphere
        ray r2(point3(0, 0, -2), vec3(0, 2, 0));
        hit_record rec2;
        expect(s.hit(r2, interval(0.001, glimpse::math::infinity), rec2) == false);

        // Ray starts inside sphere
        ray r3(point3(0, 0, 0), vec3(0, 0, 1));
        hit_record rec3;
        expect(s.hit(r3, interval(0.001, glimpse::math::infinity), rec3) == true);
        expect(rec3.t == 1.0_d);
        expect(rec3.p.z() == 1.0_d);
        expect(rec3.normal.z() == -1.0_d);
        expect(rec3.front_face == false);
      }

      // Test moving sphere hit detection
      {
        point3 center1(0, 0, 0);
        point3 center2(0, 0, 10);
        double radius = 1.0;
        auto mat = make_shared<lambertian>(color(0.5, 0.5, 0.5));
        sphere s(center1, center2, radius, mat);

        // Ray at time 0 - sphere should be at center1
        ray r1(point3(0, 0, -2), vec3(0, 0, 1), 0.0);
        hit_record rec1;
        expect(s.hit(r1, interval(0.001, glimpse::math::infinity), rec1) == true);
        expect(rec1.p.z() == -1.0_d) << "Sphere should be at initial position at t=0";

        // Ray at time 1 - sphere should be at center2
        ray r2(point3(0, 0, 8), vec3(0, 0, 1), 1.0);
        hit_record rec2;
        expect(s.hit(r2, interval(0.001, glimpse::math::infinity), rec2) == true);
        expect(rec2.p.z() == 9.0_d) << "Sphere should be at final position at t=1";

        // Ray at time 0.5 - sphere should be halfway between
        ray r3(point3(0, 0, 3), vec3(0, 0, 1), 0.5);
        hit_record rec3;
        expect(s.hit(r3, interval(0.001, glimpse::math::infinity), rec3) == true);
        expect(rec3.p.z() == 4.0_d) << "Sphere should be halfway at t=0.5";
      }
    };

    skip / "uv_mapping"_test = [] {
      point3 center(0, 0, 0);
      double radius = 1.0;
      auto mat = make_shared<lambertian>(color(0.5, 0.5, 0.5));
      sphere s(center, radius, mat);

      // Hit sphere at different positions to test UV mapping
      ray r1(point3(0, 0, -2), vec3(1, 0, 0));  // Hit at (0,0,-1) - back of sphere
      hit_record rec1;
      s.hit(r1, interval(0.001, glimpse::math::infinity), rec1);
      expect(rec1.u == 0.75_d) << "UV mapping for back of sphere";
      expect(rec1.v == 0.5_d);

      ray r2(point3(-2, 0, 0), vec3(1, 0, 0));  // Hit at (-1,0,0) - left side
      hit_record rec2;
      s.hit(r2, interval(0.001, glimpse::math::infinity), rec2);
      expect(rec2.u == 0.0_d) << "UV mapping for left side";
      expect(rec2.v == 0.5_d);

      ray r3(point3(0, -2, 0), vec3(0, 1, 0));  // Hit at (0,-1,0) - bottom
      hit_record rec3;
      s.hit(r3, interval(0.001, glimpse::math::infinity), rec3);
      expect(rec3.u == 0.5_d) << "UV mapping for bottom";
      expect(rec3.v == 0.0_d);
    };

    "pdf_value"_test = [] {
      point3 center(0, 0, 0);
      double radius = 1.0;
      auto mat = make_shared<lambertian>(color(0.5, 0.5, 0.5));
      sphere s(center, radius, mat);

      // Test pdf_value from a point outside the sphere
      point3 origin(0, 0, -3);
      vec3 direction(0, 0, 1);  // Pointing at sphere center

      double pdf_val = s.pdf_value(origin, direction);
      expect(pdf_val > 0.0_d) << "PDF value should be positive for ray hitting sphere";

      // PDF should be 1 / solid angle
      auto dist_squared = 9.0;  // Distance squared = 3^2
      auto cos_theta_max = std::sqrt(1 - radius * radius / dist_squared);
      auto solid_angle = 2 * glimpse::math::pi * (1 - cos_theta_max);
      auto expected_pdf = 1.0 / solid_angle;

      expect(std::abs(pdf_val - expected_pdf) < 0.0001_d) << "PDF value should match calculation";

      // Test pdf_value for ray that misses sphere
      vec3 miss_direction(0, 2, 0);  // Points away from sphere
      double miss_pdf = s.pdf_value(origin, miss_direction);
      expect(miss_pdf == 0.0_d) << "PDF should be 0 for ray missing sphere";
    };

    "random_direction"_test = [] {
      point3 center(0, 0, 0);
      double radius = 1.0;
      auto mat = make_shared<lambertian>(color(0.5, 0.5, 0.5));
      sphere s(center, radius, mat);

      point3 origin(0, 0, -3);

      // Test multiple random samples
      for (int i = 0; i < 50; i++) {
        vec3 random_dir = s.random(origin);

        // Direction should be unit vector
        expect(std::abs(random_dir.length() - 1.0) < 0.0001_d) << "Random direction should be unit vector";

        // Create ray in this direction and test if it hits the sphere
        ray test_ray(origin, random_dir);
        hit_record rec;
        expect(s.hit(test_ray, interval(0.001, glimpse::math::infinity), rec)) << "Random direction should hit sphere";
      }
    };
  };
}