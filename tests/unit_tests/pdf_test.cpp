#include "core/pdf.h"

#include "../test_cfg.h"

using namespace glimpse;

void pdf_test() {
  using namespace boost::ut;

  "pdf"_test = [] {
    "sphere_pdf"_test = [] {
      sphere_pdf pdf;

      // Test value method - should return 1/(4π) for any direction
      vec3 dir(1.0, 0.0, 0.0);
      expect(pdf.value(dir) == 1.0 / (4 * glimpse::math::pi)) << "Sphere PDF value should be 1/(4π)";

      // Test generate method - should return a unit vector
      vec3 generated = pdf.generate();
      expect(std::abs(generated.length() - 1.0) < 0.0001_d) << "Generated vector should have unit length";
    };

    "cosine_pdf"_test = [] {
      // Test with normal in z direction
      vec3 normal(0.0, 0.0, 1.0);
      cosine_pdf pdf(normal);

      // Test value method with various directions
      vec3 same_dir(0.0, 0.0, 1.0);
      expect(pdf.value(same_dir) == 1.0 / glimpse::math::pi) << "Cosine PDF value in normal direction should be 1/π";

      vec3 perp_dir(1.0, 0.0, 0.0);
      expect(pdf.value(perp_dir) == 0.0_d) << "Cosine PDF value in perpendicular direction should be 0";

      vec3 diagonal_dir(0.0, 0.707, 0.707);  // ~45 degrees
      expect(std::abs(pdf.value(diagonal_dir) - 0.707 / glimpse::math::pi) < 0.001_d)
          << "Cosine PDF value at 45° should be cos(45°)/π";

      // Test generate method - should return a vector with positive z component
      vec3 generated = pdf.generate();
      expect(dot(generated, normal) > 0.0_d) << "Generated vector should have positive dot product with normal";
      expect(std::abs(generated.length() - 1.0) < 0.0001_d) << "Generated vector should have unit length";
    };

    "hittable_pdf"_test = [] {
      auto empty_material = shared_ptr<material>();
      // Create a single sphere to test the hittable_pdf
      hittable_list world;
      auto sphere_center = point3(0, 0, 0);
      auto sphere_radius = 1.0;
      world.add(make_shared<sphere>(sphere_center, sphere_radius, empty_material));

      // Create a PDF for sampling this sphere from a point outside
      point3 origin(0, 0, 5);
      hittable_pdf pdf(world, origin);

      // Test generate method - should point roughly toward the sphere
      vec3 generated = pdf.generate();
      vec3 expected_direction = unit_vector(sphere_center - origin);

      // The generated vector should point roughly toward the sphere
      expect(dot(generated, expected_direction) > 0.0_d) << "Generated vector should point generally toward the sphere";

      // Test value method
      double pdf_value = pdf.value(generated);
      expect(pdf_value > 0.0_d) << "PDF value should be positive for valid direction";
    };

    "mixture_pdf"_test = [] {
      // Create two PDFs to mix
      auto pdf1 = std::make_shared<sphere_pdf>();

      vec3 normal(0.0, 0.0, 1.0);
      auto pdf2 = std::make_shared<cosine_pdf>(normal);

      mixture_pdf mixed(pdf1, pdf2);

      // Test value method - should be average of the two component PDFs
      vec3 dir(0.0, 0.0, 1.0);
      double expected_value = 0.5 * pdf1->value(dir) + 0.5 * pdf2->value(dir);
      expect(mixed.value(dir) == expected_value) << "Mixture PDF value should be average of component values";

      // Test generate method - multiple calls should eventually generate samples from both PDFs
      bool got_sphere_sample = false;
      bool got_cosine_sample = false;

      // We'll run multiple times to increase the chance of seeing both PDFs
      for (int i = 0; i < 50 && !(got_sphere_sample && got_cosine_sample); ++i) {
        vec3 sample = mixed.generate();

        // Check if this sample is more likely from the sphere PDF
        // (For this simple test, we're assuming samples with z≈0 are more likely from sphere_pdf)
        if (std::abs(sample.z()) < 0.2) {
          got_sphere_sample = true;
        }
        // Check if this sample is more likely from the cosine PDF
        // (Samples with high z component are more likely from cosine_pdf)
        else if (sample.z() > 0.8) {
          got_cosine_sample = true;
        }
      }

      expect(got_sphere_sample) << "Mixture PDF should generate samples from first component";
      expect(got_cosine_sample) << "Mixture PDF should generate samples from second component";
    };
  };
}