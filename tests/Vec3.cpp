#include "gtest/gtest.h"
//
#include "core/vec3.h"

// Test the default constructor
TEST(Vec3Test, DefaultConstructor) {
  vec3 v;
  EXPECT_EQ(v.x(), 0);
  EXPECT_EQ(v.y(), 0);
  EXPECT_EQ(v.z(), 0);
}

// Test the parameterized constructor
TEST(Vec3Test, ParameterizedConstructor) {
  vec3 v(1.0, 2.0, 3.0);
  EXPECT_EQ(v.x(), 1.0);
  EXPECT_EQ(v.y(), 2.0);
  EXPECT_EQ(v.z(), 3.0);
}

// Test the length function
TEST(Vec3Test, Length) {
  vec3 v(1.0, 2.0, 2.0);
  EXPECT_EQ(v.length(), 3.0);
}

// Test the length_squared function
TEST(Vec3Test, LengthSquared) {
  vec3 v(1.0, 2.0, 2.0);
  EXPECT_EQ(v.length_squared(), 9.0);
}

// Test the addition operator
TEST(Vec3Test, Addition) {
  vec3 v1(1.0, 2.0, 3.0);
  vec3 v2(4.0, 5.0, 6.0);
  vec3 v3 = v1 + v2;
  EXPECT_EQ(v3.x(), 5.0);
  EXPECT_EQ(v3.y(), 7.0);
  EXPECT_EQ(v3.z(), 9.0);
}

// Test the subtraction operator
TEST(Vec3Test, Subtraction) {
  vec3 v1(4.0, 5.0, 6.0);
  vec3 v2(1.0, 2.0, 3.0);
  vec3 v3 = v1 - v2;
  EXPECT_EQ(v3.x(), 3.0);
  EXPECT_EQ(v3.y(), 3.0);
  EXPECT_EQ(v3.z(), 3.0);
}

// Test the multiplication operator (scalar)
TEST(Vec3Test, MultiplicationScalar) {
  vec3 v(1.0, 2.0, 3.0);
  vec3 v2 = v * 2.0;
  EXPECT_EQ(v2.x(), 2.0);
  EXPECT_EQ(v2.y(), 4.0);
  EXPECT_EQ(v2.z(), 6.0);
}

// Test the division operator (scalar)
TEST(Vec3Test, DivisionScalar) {
  vec3 v(2.0, 4.0, 6.0);
  vec3 v2 = v / 2.0;
  EXPECT_EQ(v2.x(), 1.0);
  EXPECT_EQ(v2.y(), 2.0);
  EXPECT_EQ(v2.z(), 3.0);
}

// Test the dot product function
TEST(Vec3Test, DotProduct) {
  vec3 v1(1.0, 2.0, 3.0);
  vec3 v2(4.0, 5.0, 6.0);
  double dot_product = dot(v1, v2);
  EXPECT_EQ(dot_product, 32.0);
}

// Test the cross product function
TEST(Vec3Test, CrossProduct) {
  vec3 v1(1.0, 2.0, 3.0);
  vec3 v2(4.0, 5.0, 6.0);
  vec3 v3 = cross(v1, v2);
  EXPECT_EQ(v3.x(), -3.0);
  EXPECT_EQ(v3.y(), 6.0);
  EXPECT_EQ(v3.z(), -3.0);
}

// Test the unit_vector function
TEST(Vec3Test, UnitVector) {
  vec3 v(1.0, 2.0, 2.0);
  vec3 unit_v = unit_vector(v);
  EXPECT_NEAR(unit_v.length(), 1.0, 1e-6);
}

// Test the reflect function
TEST(Vec3Test, Reflect) {
  vec3 v(1.0, -1.0, 0.0);
  vec3 n(0.0, 1.0, 0.0);
  vec3 reflected = reflect(v, n);
  EXPECT_EQ(reflected.x(), 1.0);
  EXPECT_EQ(reflected.y(), 1.0);
  EXPECT_EQ(reflected.z(), 0.0);
}

// Test the refract function
TEST(Vec3Test, Refract) {
  vec3 uv(1.0, -1.0, 0.0);
  vec3 n(0.0, 1.0, 0.0);
  double etai_over_etat = 0.5;
  vec3 refracted = refract(uv, n, etai_over_etat);
  EXPECT_NEAR(refracted.x(), 0.5, 1e-6);
  EXPECT_NEAR(refracted.y(), -0.866025, 1e-6);
  EXPECT_NEAR(refracted.z(), 0.0, 1e-6);
}

// Test the random_in_unit_sphere function
TEST(Vec3Test, RandomInUnitSphere) {
  vec3 random_vec = random_in_unit_sphere();
  EXPECT_LE(random_vec.length_squared(), 1.0);
}

// Test the random_unit_vector function
TEST(Vec3Test, RandomUnitVector) {
  vec3 random_vec = random_unit_vector();
  EXPECT_NEAR(random_vec.length(), 1.0, 1e-6);
}

// Test the random_in_hemisphere function
TEST(Vec3Test, RandomInHemisphere) {
  vec3 normal(0.0, 1.0, 0.0);
  vec3 random_vec = random_in_hemisphere(normal);
  EXPECT_GE(dot(random_vec, normal), 0.0);
}

// Test the random_in_unit_disk function
TEST(Vec3Test, RandomInUnitDisk) {
  vec3 random_vec = random_in_unit_disk();
  EXPECT_LE(random_vec.length_squared(), 1.0);
}

// Test the sample_square function
TEST(Vec3Test, SampleSquare) {
  vec3 random_vec = sample_square();
  EXPECT_GE(random_vec.x(), -0.5);
  EXPECT_LE(random_vec.x(), 0.5);
  EXPECT_GE(random_vec.y(), -0.5);
  EXPECT_LE(random_vec.y(), 0.5);
  EXPECT_EQ(random_vec.z(), 0.0);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}