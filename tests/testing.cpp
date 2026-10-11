#define BOOST_UT_DISABLE_MODULE
#include "boost/ut.hpp"  // import boost.ut;
#include "test_cfg.h"

#include <filesystem>

// Every test file registers its suites itself (`static ut::suite<"name"> ... = [] { ... };`), so there is no list
// to keep in sync here. The e2e suite (tests/e2e/) is not registered: it is rewritten in Stage 1 (01-03).

int main(int argc, char** argv) {
  // argv[1] filters top-level test names, e.g. "camera*".
  const auto filter = argc > 1 ? argv[1] : "*";
  ut::cfg<ut::override> = ut::options{.filter = filter};

  // Scratch space for tests that write files, inside the build tree (see CMakeLists.txt).
  std::filesystem::create_directories(GLIMPSE_TEST_OUTPUT_DIR);

  // Run all registered suites now (after the filter is set). The runner prints the summary when it is destroyed.
  const bool failed = ut::cfg<ut::override>.run();
  return failed ? 1 : 0;
}
