#include "glimpse/shapes.h"

#include "glimpse/materials.h"

//
#include "../test_cfg.h"

using namespace glimpse;

static ut::suite<"sphere"> sphereSuite = [] {
  using namespace boost::ut;

  "sphere"_test = [] {
    "construction"_test = [] {
      // Test stationary sphere construction
      {
        Point3 center(1, 2, 3);
        double radius = 2.0;
        auto mat = make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
        Sphere s(center, radius, mat);

        // Test bounding box
        auto bbox = s.boundingBox();
        expect(bbox.x.min == -1.0_d);
        expect(bbox.x.max == 3.0_d);
        expect(bbox.y.min == 0.0_d);
        expect(bbox.y.max == 4.0_d);
        expect(bbox.z.min == 1.0_d);
        expect(bbox.z.max == 5.0_d);
      }

      // Test moving sphere construction
      {
        Point3 center1(1, 2, 3);
        Point3 center2(4, 5, 6);
        double radius = 2.0;
        auto mat = make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
        Sphere s(center1, center2, radius, mat);

        // Test bounding box encompasses both positions
        auto bbox = s.boundingBox();
        expect(bbox.x.min == -1.0_d);
        expect(bbox.x.max == 6.0_d);
        expect(bbox.y.min == 0.0_d);
        expect(bbox.y.max == 7.0_d);
        expect(bbox.z.min == 1.0_d);
        expect(bbox.z.max == 8.0_d);
      }

      // Test zero/negative radius
      {
        Point3 center(1, 2, 3);
        auto mat = make_shared<Lambertian>(Color(0.5, 0.5, 0.5));

        Sphere sZero(center, 0, mat);
        Sphere sNegative(center, -1, mat);

        // Both should have radius = 0 due to std::fmax(0, radius)
        auto bboxZero = sZero.boundingBox();
        expect(bboxZero.x.min == 1.0_d);
        expect(bboxZero.x.max == 1.0_d);

        auto bboxNegative = sNegative.boundingBox();
        expect(bboxNegative.x.min == 1.0_d);
        expect(bboxNegative.x.max == 1.0_d);
      }
    };

    "hit_detection"_test = [] {
      // Test stationary sphere hit detection
      {
        Point3 center(0, 0, 0);
        double radius = 1.0;
        auto mat = make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
        Sphere s(center, radius, mat);

        // Ray hits sphere directly
        Ray r(Point3(0, 0, -2), Vec3(0, 0, 1));
        HitRecord rec;
        expect(s.hit(r, Interval(0.001, glimpse::math::infinity), rec) == true);
        expect(rec.t == 1.0_d);
        expect(rec.p.z() == -1.0_d);
        expect(rec.normal.z() == -1.0_d);
        expect(rec.frontFace == true);

        // Ray misses sphere
        Ray r2(Point3(0, 0, -2), Vec3(0, 2, 0));
        HitRecord rec2;
        expect(s.hit(r2, Interval(0.001, glimpse::math::infinity), rec2) == false);

        // Ray starts inside sphere
        Ray r3(Point3(0, 0, 0), Vec3(0, 0, 1));
        HitRecord rec3;
        expect(s.hit(r3, Interval(0.001, glimpse::math::infinity), rec3) == true);
        expect(rec3.t == 1.0_d);
        expect(rec3.p.z() == 1.0_d);
        expect(rec3.normal.z() == -1.0_d);
        expect(rec3.frontFace == false);
      }

      // Test moving sphere hit detection
      {
        Point3 center1(0, 0, 0);
        Point3 center2(0, 0, 10);
        double radius = 1.0;
        auto mat = make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
        Sphere s(center1, center2, radius, mat);

        // Ray at time 0 - sphere should be at center1
        Ray r1(Point3(0, 0, -2), Vec3(0, 0, 1), 0.0);
        HitRecord rec1;
        expect(s.hit(r1, Interval(0.001, glimpse::math::infinity), rec1) == true);
        expect(rec1.p.z() == -1.0_d) << "Sphere should be at initial position at t=0";

        // Ray at time 1 - sphere should be at center2
        Ray r2(Point3(0, 0, 8), Vec3(0, 0, 1), 1.0);
        HitRecord rec2;
        expect(s.hit(r2, Interval(0.001, glimpse::math::infinity), rec2) == true);
        expect(rec2.p.z() == 9.0_d) << "Sphere should be at final position at t=1";

        // Ray at time 0.5 - sphere should be halfway between
        Ray r3(Point3(0, 0, 3), Vec3(0, 0, 1), 0.5);
        HitRecord rec3;
        expect(s.hit(r3, Interval(0.001, glimpse::math::infinity), rec3) == true);
        expect(rec3.p.z() == 4.0_d) << "Sphere should be halfway at t=0.5";
      }
    };

    skip / "uv_mapping"_test = [] {
      Point3 center(0, 0, 0);
      double radius = 1.0;
      auto mat = make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
      Sphere s(center, radius, mat);

      // Hit sphere at different positions to test UV mapping
      Ray r1(Point3(0, 0, -2), Vec3(1, 0, 0));  // Hit at (0,0,-1) - back of sphere
      HitRecord rec1;
      s.hit(r1, Interval(0.001, glimpse::math::infinity), rec1);
      expect(rec1.u == 0.75_d) << "UV mapping for back of sphere";
      expect(rec1.v == 0.5_d);

      Ray r2(Point3(-2, 0, 0), Vec3(1, 0, 0));  // Hit at (-1,0,0) - left side
      HitRecord rec2;
      s.hit(r2, Interval(0.001, glimpse::math::infinity), rec2);
      expect(rec2.u == 0.0_d) << "UV mapping for left side";
      expect(rec2.v == 0.5_d);

      Ray r3(Point3(0, -2, 0), Vec3(0, 1, 0));  // Hit at (0,-1,0) - bottom
      HitRecord rec3;
      s.hit(r3, Interval(0.001, glimpse::math::infinity), rec3);
      expect(rec3.u == 0.5_d) << "UV mapping for bottom";
      expect(rec3.v == 0.0_d);
    };

    "pdf_value"_test = [] {
      Point3 center(0, 0, 0);
      double radius = 1.0;
      auto mat = make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
      Sphere s(center, radius, mat);

      // Test pdfValue from a point outside the sphere
      Point3 origin(0, 0, -3);
      Vec3 direction(0, 0, 1);  // Pointing at sphere center

      double pdfVal = s.pdfValue(origin, direction);
      expect(pdfVal > 0.0_d) << "PDF value should be positive for ray hitting sphere";

      // PDF should be 1 / solid angle
      auto distSquared = 9.0;  // Distance squared = 3^2
      auto cosThetaMax = std::sqrt(1 - radius * radius / distSquared);
      auto solidAngle = 2 * glimpse::math::pi * (1 - cosThetaMax);
      auto expectedPdf = 1.0 / solidAngle;

      expect(std::abs(pdfVal - expectedPdf) < 0.0001_d) << "PDF value should match calculation";

      // Test pdfValue for ray that misses sphere
      Vec3 missDirection(0, 2, 0);  // Points away from sphere
      double missPdf = s.pdfValue(origin, missDirection);
      expect(missPdf == 0.0_d) << "PDF should be 0 for ray missing sphere";
    };

    "random_direction"_test = [] {
      Point3 center(0, 0, 0);
      double radius = 1.0;
      auto mat = make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
      Sphere s(center, radius, mat);

      Point3 origin(0, 0, -3);

      // Test multiple random samples
      for (int i = 0; i < 50; i++) {
        Vec3 randomDir = s.random(origin);

        // Direction should be unit vector
        expect(std::abs(randomDir.length() - 1.0) < 0.0001_d) << "Random direction should be unit vector";

        // Create ray in this direction and test if it hits the sphere
        Ray testRay(origin, randomDir);
        HitRecord rec;
        expect(s.hit(testRay, Interval(0.001, glimpse::math::infinity), rec)) << "Random direction should hit sphere";
      }
    };
  };
};