#define BOOST_UT_DISABLE_MODULE
#include "boost/ut.hpp"  // import boost.ut;
#include "test_cfg.h"

constexpr auto sum(auto... values) { return (values + ...); }

void vec3_test();
void ray_test();
void color_test();
void film_test();
void aabb_test();
void texture_test();
void image_test();
void interval_test();
void onb_test();

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

  /////// Tests
  vec3_test();
  ray_test();
  color_test();
  film_test();
  aabb_test();
  texture_test();
  image_test();
  interval_test();
  onb_test();

  return 0;
}