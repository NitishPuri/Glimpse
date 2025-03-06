
#define BOOST_UT_DISABLE_MODULE
#include "../src/core/vec3.h"
#include "boost/ut.hpp"

void tests_vec3() {
  using namespace boost::ut;

  "vec3_construction"_test = [] {
    vec3 v1;
    expect(v1.x() == 0.0_d);
    expect(v1.y() == 0.0_d);
    expect(v1.z() == 0.0_d);

    vec3 v2(1.0, 2.0, 3.0);
    expect(v2.x() == 1.0_d);
    expect(v2.y() == 2.0_d);
    expect(v2.z() == 3.0_d);
  };

  "vec3_operations"_test = [] {
    vec3 v(3.0, 4.0, 0.0);
    expect(v.length() == 5.0_d) << "Length calculation failed";
    expect(v.length_squared() == 25.0_d);

    vec3 normalized = unit_vector(v);
    expect(normalized.length() == 1.0_d) << "Unit vector length should be 1";
  };

  "vec3_operators"_test = [] {
    vec3 v1(1.0, 2.0, 3.0);
    vec3 v2(2.0, 3.0, 4.0);

    auto sum = v1 + v2;
    expect(sum.x() == 3.0_d);
    expect(sum.y() == 5.0_d);
    expect(sum.z() == 7.0_d);

    auto scaled = v1 * 2.0;
    expect(scaled.x() == 2.0_d);
    expect(scaled.y() == 4.0_d);
    expect(scaled.z() == 6.0_d);
  };

  "vec3_dot_product"_test = [] {
    vec3 v1(1.0, 0.0, 0.0);
    vec3 v2(0.0, 1.0, 0.0);
    expect(dot(v1, v2) == 0.0_d) << "Perpendicular vectors dot product should be 0";

    vec3 v3(2.0, 3.0, 4.0);
    expect(dot(v3, v3) == v3.length_squared());
  };

  "vec3_cross_product"_test = [] {
    vec3 v1(1.0, 0.0, 0.0);
    vec3 v2(0.0, 1.0, 0.0);
    vec3 result = cross(v1, v2);
    expect(result.x() == 0.0_d);
    expect(result.y() == 0.0_d);
    expect(result.z() == 1.0_d);
  };

  "vec3_reflection"_test = [] {
    vec3 v(1.0, -1.0, 0.0);
    vec3 n(0.0, 1.0, 0.0);
    vec3 reflected = reflect(v, n);
    expect(reflected.x() == 1.0_d);
    expect(reflected.y() == 1.0_d);
    expect(reflected.z() == 0.0_d);
  };
};