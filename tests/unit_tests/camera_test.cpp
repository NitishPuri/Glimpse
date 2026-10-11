#include "glimpse/camera.h"

#include "../test_cfg.h"

using namespace glimpse;

static ut::suite<"camera"> cameraSuite = [] {
  using namespace boost::ut;

  "camera"_test = [] {
    "default_construction"_test = [] {
      Camera cam;
      expect(cam.aspectRatio == 16.0 / 9.0);
      expect(cam.imageWidth == 800_i);
      expect(cam.samplesPerPixel == 100_i);
      expect(cam.maxDepth == 50_i);
      expect(cam.vfov == 60.0_f);
      expect(cam.defocusAngle == 0.1_f);
      expect(cam.focusDistance == 10.0_f);
    };

    "initialization"_test = [] {
      Camera cam;
      cam.initialize();
      expect(cam.imageHeight > 0_i);
      expect(cam.sqrtSpp == 10_i);  // sqrt of default 100 samples
      expect(cam.pixelSamplesScale == 1.0 / (10.0 * 10.0));
      expect(cam.recipSqrtSpp == 0.1_d);
    };

    "ray_generation"_test = [] {
      Camera cam;
      cam.lookfrom = Point3(0, 0, 0);
      cam.lookat = Point3(0, 0, -1);
      cam.vup = Vec3(0, 1, 0);
      cam.defocusAngle = 0.0;
      cam.initialize();

      // Get ray for center of viewport
      Ray centerRay = cam.getRay(0.5, 0.5);

      // Ray should originate at lookfrom ( when defocus is zero )
      expect(centerRay.origin() == cam.lookfrom) << centerRay.origin() << cam.lookfrom;

      // Ray direction should point approximately toward lookat
      Vec3 expectedDirection = unitVector(cam.lookat - cam.lookfrom);
      Vec3 actualDirection = unitVector(centerRay.direction());

      // Direction should be approximately correct (dot product close to 1)
      expect(dot(expectedDirection, actualDirection) > 0.9_d);
    };

    "defocus_blur"_test = [] {
      Camera cam;
      cam.defocusAngle = 2.0;
      cam.focusDistance = 10.0;
      cam.lookfrom = Point3(0, 0, 0);
      cam.lookat = Point3(0, 0, -1);
      cam.initialize();

      // With defocus blur enabled, rays should originate from the defocus disk
      Ray r1 = cam.getRay(0.5, 0.5);
      Ray r2 = cam.getRay(0.5, 0.5);

      // Two rays should have different origins due to the defocus disk sampling
      expect(r1.origin() != r2.origin());
    };

    "camera_movement"_test = [] {
      Camera cam;
      Point3 originalLookfrom = Point3(0, 0, 0);
      Point3 originalLookat = Point3(0, 0, -1);
      cam.lookfrom = originalLookfrom;
      cam.lookat = originalLookat;
      cam.initialize();

      // Test fly mode
      cam.fly(1.0, 0.0);  // Move right
      expect(cam.lookfrom.x() > originalLookfrom.x());
      expect(cam.lookat.x() > originalLookat.x());

      // Reset and test orbit
      cam.lookfrom = Point3(0, 0, 5);
      cam.lookat = Point3(0, 0, 0);
      cam.initialize();
      Point3 preOrbitLookfrom = cam.lookfrom;
      cam.orbit(0.1f, 0);  // Orbit around lookat

      // Lookfrom should change but maintain same distance to lookat
      expect(cam.lookfrom != preOrbitLookfrom);
      expect((cam.lookfrom - cam.lookat).length() == (preOrbitLookfrom - cam.lookat).length());
    };
  };
};