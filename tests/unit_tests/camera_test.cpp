#include "core/camera.h"

#include "../test_cfg.h"

using namespace glimpse;

void camera_test() {
  using namespace boost::ut;

  "camera"_test = [] {
    "default_construction"_test = [] {
      camera cam;
      expect(cam.aspect_ratio == 16.0 / 9.0);
      expect(cam.image_width == 800_i);
      expect(cam.samples_per_pixel == 100_i);
      expect(cam.max_depth == 50_i);
      expect(cam.vfov == 60.0_f);
      expect(cam.defocus_angle == 0.1_f);
      expect(cam.focus_distance == 10.0_f);
    };

    "initialization"_test = [] {
      camera cam;
      cam.initialize();
      expect(cam.image_height > 0_i);
      expect(cam.sqrt_spp == 10_i);  // sqrt of default 100 samples
      expect(cam.pixel_samples_scale == 1.0 / (10.0 * 10.0));
      expect(cam.recip_sqrt_spp == 0.1_d);
    };

    "ray_generation"_test = [] {
      camera cam;
      cam.lookfrom = point3(0, 0, 0);
      cam.lookat = point3(0, 0, -1);
      cam.vup = vec3(0, 1, 0);
      cam.defocus_angle = 0.0;
      cam.initialize();

      // Get ray for center of viewport
      ray center_ray = cam.get_ray(0.5, 0.5);

      // Ray should originate at lookfrom ( when defocus is zero )
      expect(center_ray.origin() == cam.lookfrom) << center_ray.origin() << cam.lookfrom;

      // Ray direction should point approximately toward lookat
      vec3 expected_direction = unit_vector(cam.lookat - cam.lookfrom);
      vec3 actual_direction = unit_vector(center_ray.direction());

      // Direction should be approximately correct (dot product close to 1)
      expect(dot(expected_direction, actual_direction) > 0.9_d);
    };

    "defocus_blur"_test = [] {
      camera cam;
      cam.defocus_angle = 2.0;
      cam.focus_distance = 10.0;
      cam.lookfrom = point3(0, 0, 0);
      cam.lookat = point3(0, 0, -1);
      cam.initialize();

      // With defocus blur enabled, rays should originate from the defocus disk
      ray r1 = cam.get_ray(0.5, 0.5);
      ray r2 = cam.get_ray(0.5, 0.5);

      // Two rays should have different origins due to the defocus disk sampling
      expect(r1.origin() != r2.origin());
    };

    "camera_movement"_test = [] {
      camera cam;
      point3 original_lookfrom = point3(0, 0, 0);
      point3 original_lookat = point3(0, 0, -1);
      cam.lookfrom = original_lookfrom;
      cam.lookat = original_lookat;
      cam.initialize();

      // Test fly mode
      cam.fly(1.0, 0.0);  // Move right
      expect(cam.lookfrom.x() > original_lookfrom.x());
      expect(cam.lookat.x() > original_lookat.x());

      // Reset and test orbit
      cam.lookfrom = point3(0, 0, 5);
      cam.lookat = point3(0, 0, 0);
      cam.initialize();
      point3 pre_orbit_lookfrom = cam.lookfrom;
      cam.orbit(0.1f, 0);  // Orbit around lookat

      // Lookfrom should change but maintain same distance to lookat
      expect(cam.lookfrom != pre_orbit_lookfrom);
      expect((cam.lookfrom - cam.lookat).length() == (pre_orbit_lookfrom - cam.lookat).length());
    };
  };
}