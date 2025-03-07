#define BOOST_UT_DISABLE_MODULE
#include "boost/ut.hpp"  // import boost.ut;
#include "test_cfg.h"

template <>
auto ut::cfg<ut::override> = ut::runner<ut::reporter<ut::printer>>{};

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
void random_test();

// End-to-end tests
void e2e_test();

void example_tests() {
  using namespace boost::ut;

  "sum"_test = [] {
    expect(sum(0) == 0_i);
    expect(sum(1, 2) == 3_i);
    expect(sum(1, 2) > 0_i and 42_i == sum(40, 2));

    int v[] = {1, 2, 3};
    expect(6_i == sum(v[0], v[1], v[2]));
    fatal(v[4] = 4);  // should fail
  };

  "basic_assertions"_test = [] {
    expect(2_i == 2);
    expect(2_i != 3) << "2 should not equal 3";

    // Using fatal assertion - test stops here if this fails
    expect(1_i == 1) << fatal << "This would abort the test if it failed";

    // Won't execute if the above fails
    expect(true);
  };

  "safe_test_example"_test = [] {
    // safe_test("division_test", [] {
    int divisor = 0;
    // This would normally crash the program
    // But safe_test will catch it

    int result = 10 / divisor;
    // abort();  // should not reach this line
    // expect(result > 0) << "Result should be positive";
    // });
  };
}

int main(int argc, char** argv) {
  // setup filter
  const auto filter = argc > 1 ? argv[1] : "*";
  ut::cfg<ut::override> = ut::options{.filter = filter};
  // ut::cfg<ut::runner<ut::reporter<ut::printer>>> = ut::options{.filter = filter};
  // ut::runner<ut::reporter<ut::printer>> = ut::options{.filter = filter};

  example_tests();

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

  // E2E
  // e2e_test();

  return 0;
}