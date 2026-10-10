#define BOOST_UT_DISABLE_MODULE
#include "boost/ut.hpp"  // import boost.ut;
#include "test_cfg.h"

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
void random_test();
void cli_test();

// End-to-end tests
void e2e_test();

int main(int argc, char** argv) {
  // setup filter
  const auto filter = argc > 1 ? argv[1] : "*";
  ut::cfg<ut::override> = ut::options{.filter = filter};
  // ut::cfg<ut::runner<ut::reporter<ut::printer>>> = ut::options{.filter = filter};
  // ut::runner<ut::reporter<ut::printer>> = ut::options{.filter = filter};

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
  random_test();
  cli_test();

  // E2E
  // e2e_test();

  return 0;
}