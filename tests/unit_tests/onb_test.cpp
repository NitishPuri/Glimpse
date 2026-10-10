#include "core/onb.h"

#include "../test_cfg.h"

using namespace glimpse;

void onb_test() {
  using namespace boost::ut;

  "onb"_test = [] {
    "construction"_test = [] {
      // Test construction with unit vectors
      {
        vec3 n(0, 0, 1);  // z-axis
        onb basis(n);

        expect(basis.w() == n) << "w vector should match input normal";
        expect(basis.u().length() == 1.0_d) << "u should be unit length";
        expect(basis.v().length() == 1.0_d) << "v should be unit length";
        expect(basis.w().length() == 1.0_d) << "w should be unit length";

        // Verify orthogonality
        expect(dot(basis.u(), basis.v()) == 0.0_d) << "u and v should be orthogonal";
        expect(dot(basis.u(), basis.w()) == 0.0_d) << "u and w should be orthogonal";
        expect(dot(basis.v(), basis.w()) == 0.0_d) << "v and w should be orthogonal";
      }

      // Test with x-axis (special case in the code)
      {
        vec3 n(1, 0, 0);  // x-axis
        onb basis(n);

        expect(basis.w() == n) << "w vector should match input normal";
        expect(basis.u().length() == 1.0_d) << "u should be unit length";
        expect(basis.v().length() == 1.0_d) << "v should be unit length";

        // Verify orthogonality
        expect(dot(basis.u(), basis.v()) == 0.0_d) << "u and v should be orthogonal";
        expect(dot(basis.u(), basis.w()) == 0.0_d) << "u and w should be orthogonal";
        expect(dot(basis.v(), basis.w()) == 0.0_d) << "v and w should be orthogonal";
      }

      // Test with non-unit vector
      {
        vec3 n(2, 3, 4);  // non-unit vector
        onb basis(n);

        expect(basis.w() == unit_vector(n)) << "w should be normalized input";
        expect(basis.u().length() == 1.0_d) << "u should be unit length";
        expect(basis.v().length() == 1.0_d) << "v should be unit length";
        expect(basis.w().length() == 1.0_d) << "w should be unit length";

        // Verify orthogonality
        expect(dot(basis.u(), basis.v()) == 0.0_d) << "u and v should be orthogonal";
        expect(dot(basis.u(), basis.w()) == 0.0_d) << "u and w should be orthogonal";
        expect(dot(basis.v(), basis.w()) == 0.0_d) << "v and w should be orthogonal";
      }
    };

    "transform"_test = [] {
      {
        // Identity case - aligned with global axes
        vec3 n(0, 0, 1);
        onb basis(n);

        vec3 v(1, 2, 3);
        vec3 transformed = basis.transform(v);

        // Since we're aligned with z-axis, u should be roughly x-axis and v roughly y-axis
        // So the transform should roughly preserve x and y and z components
        expect(transformed.x() != 0.0_d);
        expect(transformed.y() != 0.0_d);
        expect(transformed.z() != 0.0_d);
        expect(transformed.length() == v.length()) << "Transformation should preserve length";
      }

      {
        // Test basic properties of transform
        vec3 n(1, 1, 1);  // 45-degree angle with each axis
        onb basis(n);

        // Transform basis vectors
        vec3 transformed_x = basis.transform(vec3(1, 0, 0));
        vec3 transformed_y = basis.transform(vec3(0, 1, 0));
        vec3 transformed_z = basis.transform(vec3(0, 0, 1));

        // Each should match the corresponding basis vector
        expect(transformed_x == basis.u()) << "Transforming (1,0,0) should yield u";
        expect(transformed_y == basis.v()) << "Transforming (0,1,0) should yield v";
        expect(transformed_z == basis.w()) << "Transforming (0,0,1) should yield w";
      }
    };
  };
}