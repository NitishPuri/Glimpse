#include "glimpse/pdf.h"

#include "../test_cfg.h"

using namespace glimpse;

static ut::suite<"pdf"> pdfSuite = [] {
  using namespace boost::ut;

  "pdf"_test = [] {
    "sphere_pdf"_test = [] {
      SpherePDF p;

      // Test value method - should return 1/(4π) for any direction
      Vec3 dir(1.0, 0.0, 0.0);
      expect(p.value(dir) == 1.0 / (4 * glimpse::math::pi)) << "Sphere PDF value should be 1/(4π)";

      // Test generate method - should return a unit vector
      Vec3 generated = p.generate();
      expect(std::abs(generated.length() - 1.0) < 0.0001_d) << "Generated vector should have unit length";
    };

    "cosine_pdf"_test = [] {
      // Test with normal in z direction
      Vec3 normal(0.0, 0.0, 1.0);
      CosinePDF p(normal);

      // Test value method with various directions
      Vec3 sameDir(0.0, 0.0, 1.0);
      expect(p.value(sameDir) == 1.0 / glimpse::math::pi) << "Cosine PDF value in normal direction should be 1/π";

      Vec3 perpDir(1.0, 0.0, 0.0);
      expect(p.value(perpDir) == 0.0_d) << "Cosine PDF value in perpendicular direction should be 0";

      Vec3 diagonalDir(0.0, 0.707, 0.707);  // ~45 degrees
      expect(std::abs(p.value(diagonalDir) - 0.707 / glimpse::math::pi) < 0.001_d)
          << "Cosine PDF value at 45° should be cos(45°)/π";

      // Test generate method - should return a vector with positive z component
      Vec3 generated = p.generate();
      expect(dot(generated, normal) > 0.0_d) << "Generated vector should have positive dot product with normal";
      expect(std::abs(generated.length() - 1.0) < 0.0001_d) << "Generated vector should have unit length";
    };

    "hittable_pdf"_test = [] {
      auto emptyMaterial = shared_ptr<Material>();
      // Create a single sphere to test the hittable_pdf
      HittableList world;
      auto sphereCenter = Point3(0, 0, 0);
      auto sphereRadius = 1.0;
      world.add(make_shared<Sphere>(sphereCenter, sphereRadius, emptyMaterial));

      // Create a PDF for sampling this sphere from a point outside
      Point3 origin(0, 0, 5);
      HittablePDF p(world, origin);

      // Test generate method - should point roughly toward the sphere
      Vec3 generated = p.generate();
      Vec3 expectedDirection = unitVector(sphereCenter - origin);

      // The generated vector should point roughly toward the sphere
      expect(dot(generated, expectedDirection) > 0.0_d) << "Generated vector should point generally toward the sphere";

      // Test value method
      double pdfValue = p.value(generated);
      expect(pdfValue > 0.0_d) << "PDF value should be positive for valid direction";
    };

    "mixture_pdf"_test = [] {
      // Create two PDFs to mix
      auto pdf1 = std::make_shared<SpherePDF>();

      Vec3 normal(0.0, 0.0, 1.0);
      auto pdf2 = std::make_shared<CosinePDF>(normal);

      MixturePDF mixed(pdf1, pdf2);

      // Test value method - should be average of the two component PDFs
      Vec3 dir(0.0, 0.0, 1.0);
      double expectedValue = 0.5 * pdf1->value(dir) + 0.5 * pdf2->value(dir);
      expect(mixed.value(dir) == expectedValue) << "Mixture PDF value should be average of component values";

      // Test generate method - multiple calls should eventually generate samples from both PDFs
      bool gotSphereSample = false;
      bool gotCosineSample = false;

      // We'll run multiple times to increase the chance of seeing both PDFs
      for (int i = 0; i < 50 && !(gotSphereSample && gotCosineSample); ++i) {
        Vec3 sample = mixed.generate();

        // Check if this sample is more likely from the sphere PDF
        // (For this simple test, we're assuming samples with z≈0 are more likely from sphere_pdf)
        if (std::abs(sample.z()) < 0.2) {
          gotSphereSample = true;
        }
        // Check if this sample is more likely from the cosine PDF
        // (Samples with high z component are more likely from cosine_pdf)
        else if (sample.z() > 0.8) {
          gotCosineSample = true;
        }
      }

      expect(gotSphereSample) << "Mixture PDF should generate samples from first component";
      expect(gotCosineSample) << "Mixture PDF should generate samples from second component";
    };
  };
};