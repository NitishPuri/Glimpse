#define BOOST_UT_DISABLE_MODULE
#include "boost/ut.hpp"  // import boost.ut;
#include "test_cfg.h"

// template <>
// auto ut::cfg<ut::override> = cfg::runner<cfg::reporter>{};

constexpr auto sum(auto... values) { return (values + ...); }

void vec3_test();
void ray_test();
void film_test();
void aabb_test();
void texture_test();
void image_test();
void test_image_roundtrip();
void interval_test();
void onb_test();
void pdf_test();
void perlin_test();
void camera_test();
void render_test();
void sphere_test();
void bvh_test();

// End-to-end tests
void e2e_test();

int main(int argc, char** argv) {
  // setup filter
  const auto filter = argc > 1 ? argv[1] : "*";
  ut::cfg<ut::override> = ut::options{.filter = filter};

  using namespace boost::ut;
  "sum"_test = [] {
    expect(sum(0) == 0_i);
    expect(sum(1, 2) == 3_i);
    expect(sum(1, 2) > 0_i and 42_i == sum(40, 2));
  };

  // Unit Tests
  vec3_test();
  ray_test();
  film_test();
  aabb_test();
  texture_test();
  image_test();
  test_image_roundtrip();
  interval_test();
  onb_test();
  pdf_test();
  perlin_test();
  camera_test();
  render_test();
  sphere_test();
  bvh_test();

  // E2E
  // e2e_test();

  return 0;
}