#include "glimpse/util/vecmath.h"

#include "../test_cfg.h"

using namespace glimpse;

static ut::suite<"onb"> onbSuite = [] {
  using namespace boost::ut;

  "onb"_test = [] {
    "construction"_test = [] {
      // Test construction with unit vectors
      {
        Vec3 n(0, 0, 1);  // z-axis
        ONB basis(n);

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
        Vec3 n(1, 0, 0);  // x-axis
        ONB basis(n);

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
        Vec3 n(2, 3, 4);  // non-unit vector
        ONB basis(n);

        expect(basis.w() == unitVector(n)) << "w should be normalized input";
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
        Vec3 n(0, 0, 1);
        ONB basis(n);

        Vec3 v(1, 2, 3);
        Vec3 transformed = basis.transform(v);

        // Since we're aligned with z-axis, u should be roughly x-axis and v roughly y-axis
        // So the transform should roughly preserve x and y and z components
        expect(transformed.x() != 0.0_d);
        expect(transformed.y() != 0.0_d);
        expect(transformed.z() != 0.0_d);
        expect(transformed.length() == v.length()) << "Transformation should preserve length";
      }

      {
        // Test basic properties of transform
        Vec3 n(1, 1, 1);  // 45-degree angle with each axis
        ONB basis(n);

        // Transform basis vectors
        Vec3 transformedX = basis.transform(Vec3(1, 0, 0));
        Vec3 transformedY = basis.transform(Vec3(0, 1, 0));
        Vec3 transformedZ = basis.transform(Vec3(0, 0, 1));

        // Each should match the corresponding basis vector
        expect(transformedX == basis.u()) << "Transforming (1,0,0) should yield u";
        expect(transformedY == basis.v()) << "Transforming (0,1,0) should yield v";
        expect(transformedZ == basis.w()) << "Transforming (0,0,1) should yield w";
      }
    };
  };
};