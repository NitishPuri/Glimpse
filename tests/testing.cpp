
#define BOOST_UT_DISABLE_MODULE
#include "boost/ut.hpp"  // import boost.ut;

namespace ut = boost::ut;

constexpr auto sum(auto... values) { return (values + ...); }

void vec3_test();
void ray_test();
void color_test();
void film_test();
void aabb_test();

namespace cfg {
class reporter : public ut::reporter<ut::printer> {
 public:
  using BaseReporter = ut::reporter<ut::printer>;

  auto on(ut::events::test_begin test) -> void { std::cout << "[TEST] Running: " << test.name << std::endl; }
  auto on(ut::events::test_skip test) -> void { std::cout << "[SKIP] Skipped: " << test.name << std::endl; }
  // auto on(ut::events::test_end test) -> void { std::cout << "[DONE] Finished: " << test.name << std::endl; }

  auto operator=(const ut::options& options) -> reporter& {
    BaseReporter::operator=({options.colors});
    return *this;
  }

  // Add this operator to handle initializer list assignments
  template <class T>
  auto operator=(std::initializer_list<T> il) -> reporter& {
    return *this;
  }

  using BaseReporter::on;
};

template <class Reporter = reporter>
class runner : public ut::runner<Reporter> {
 public:
  using BaseRunner = ut::runner<Reporter>;

  auto& operator=(const ut::options& options) {
    this->BaseRunner::operator=(options);
    return *this;
  }
};
}  // namespace cfg

template <>
auto ut::cfg<ut::override> = cfg::runner<cfg::reporter>{};

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

  return 0;
}