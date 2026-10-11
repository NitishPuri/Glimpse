#pragma once

#define BOOST_UT_DISABLE_MODULE
#include "boost/ut.hpp"  // import boost.ut;
namespace ut = boost::ut;

// The one runner every test file reports to. It must be `inline` and visible before any test file uses
// ut::cfg, otherwise each file instantiates its own copy (ODR violation; GCC refuses to link, MSVC didn't).
template <>
inline auto ut::cfg<ut::override> = ut::runner<ut::reporter<ut::printer>>{};

#include <string>

// Where tests write files: build/<cfg>/testOutput (GLIMPSE_TEST_OUTPUT_DIR comes from CMake), not the repo root.
inline std::string testOutputPath(const std::string& file) { return std::string(GLIMPSE_TEST_OUTPUT_DIR) + "/" + file; }
