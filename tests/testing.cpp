// #include <boost/ut.hpp>  // import boost.ut;
#define BOOST_UT_DISABLE_MODULE
#include "boost/ut.hpp"  // import boost.ut;

constexpr auto sum(auto... values) { return (values + ...); }

void vec3_test();
void aabb_test();

int main(int argc, char** argv) {
  using namespace boost::ut;

  // setup filter
  const auto filter = argc > 1 ? argv[1] : "";

  cfg<override> = {.filter = filter};

  "sum"_test = [] {
    expect(sum(0) == 0_i);
    expect(sum(1, 2) == 3_i);
    expect(sum(1, 2) > 0_i and 42_i == sum(40, 2));
  };

  /////// Tests
  vec3_test();
  aabb_test();

  return 0;
}