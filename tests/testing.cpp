// #include <boost/ut.hpp>  // import boost.ut;
#define BOOST_UT_DISABLE_MODULE
#include "boost/ut.hpp"  // import boost.ut;

constexpr auto sum(auto... values) { return (values + ...); }

void tests_vec3();

int main() {
  using namespace boost::ut;

  "sum"_test = [] {
    expect(sum(0) == 0_i);
    expect(sum(1, 2) == 3_i);
    expect(sum(1, 2) > 0_i and 42_i == sum(40, 2));
  };

  /////// Tests
  tests_vec3();

  return 0;
}