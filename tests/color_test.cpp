#include "core/color.h"

#define BOOST_UT_DISABLE_MODULE
#include "boost/ut.hpp"

void color_test() {
  using namespace boost::ut;

  "ImageColor RGB constructor"_test = [] {
    ImageColor c(128, 64, 32);
    expect(c.rgb[0] == 128_uc);
    expect(c.rgb[1] == 64_uc);
    expect(c.rgb[2] == 32_uc);
  };

  "ImageColor from normal color vector"_test = [] {
    ImageColor c(color(0.5, 0.25, 0.125));
    // Gamma correction: sqrt(0.5) ≈ 0.707, sqrt(0.25) = 0.5, sqrt(0.125) ≈ 0.354
    // Then multiply by 256 and clamp
    expect(c.rgb[0] == 181_uc);  // ≈ 0.707 * 256
    expect(c.rgb[1] == 128_uc);  // = 0.5 * 256
    expect(c.rgb[2] == 90_uc);   // ≈ 0.354 * 256
  };

  "ImageColor from NaN values"_test = [] {
    ImageColor c(color(NAN, NAN, NAN));
    expect(c.rgb[0] == 0_uc);
    expect(c.rgb[1] == 0_uc);
    expect(c.rgb[2] == 0_uc);
  };

  "ImageColor from out of range values"_test = [] {
    ImageColor c(color(1.5, -0.5, 2.0));
    expect(c.rgb[0] == 255_uc);  // Clamped to 0.999 -> 255
    expect(c.rgb[1] == 0_uc);    // Negative becomes 0
    expect(c.rgb[2] == 255_uc);  // Clamped to 0.999 -> 255
  };
}
