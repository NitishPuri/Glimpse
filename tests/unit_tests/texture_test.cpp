#include "glimpse/textures.h"

#include "../test_cfg.h"

using namespace glimpse;

static ut::suite<"texture"> textureSuite = [] {
  using namespace boost::ut;
  using namespace std;

  "solid_color"_test = [] {
    Color c(0.8, 0.3, 0.2);
    SolidColor tex(c);

    Point3 point(1.0, 2.0, 3.0);
    Color result = tex.value(0.5, 0.5, point);

    expect(that % result.x() == 0.8);
    expect(that % result.y() == 0.3);
    expect(that % result.z() == 0.2);
  };

  "checker_texture"_test = [] {
    Color c1(1.0, 0.0, 0.0);  // Red
    Color c2(0.0, 1.0, 0.0);  // Green
    double scale = 1.0;
    CheckerTexture tex(scale, c1, c2);

    // Points that should return "even" texture (red)
    Point3 evenPoint1(0.5, 0.5, 0.5);  // 0+0+0 is even
    Point3 evenPoint2(1.5, 1.5, 1.5);  // 1+1+1 is odd

    // Points that should return "odd" texture (green)
    Point3 oddPoint1(0.5, 0.5, 1.5);  // 0+0+1 is odd
    Point3 oddPoint2(1.5, 0.5, 0.5);  // 1+0+0 is odd

    expect(that % tex.value(0.0, 0.0, evenPoint1) == c1);
    expect(that % tex.value(0.0, 0.0, evenPoint2) == c2);
    expect(that % tex.value(0.0, 0.0, oddPoint1) == c2);
    expect(that % tex.value(0.0, 0.0, oddPoint2) == c2);
  };

  "noise_texture"_test = [] {
    double scale = 4.0;
    Color baseColor(0.5, 0.5, 0.5);
    NoiseTexture tex(scale, baseColor);

    Point3 p1(1.0, 2.0, 3.0);
    Point3 p2(1.0, 2.0, 3.0);  // Same point
    Point3 p3(4.0, 5.0, 6.0);  // Different point

    // Same point should produce the same color
    Color c1 = tex.value(0.5, 0.5, p1);
    Color c2 = tex.value(0.5, 0.5, p2);
    expect(that % c1 == c2);

    // Different point should (likely) produce different color
    Color c3 = tex.value(0.5, 0.5, p3);
    expect(that % (c1.x() != c3.x() || c1.y() != c3.y() || c1.z() != c3.z()));

    // Check basic bounds - values should be reasonable for sine-based functions
    expect(c1.x() >= 0.0_d);
    expect(c1.y() >= 0.0_d);
    expect(c1.z() >= 0.0_d);
    expect(c1.x() <= 1.0_d);
    expect(c1.y() <= 1.0_d);
    expect(c1.z() <= 1.0_d);
  };
};