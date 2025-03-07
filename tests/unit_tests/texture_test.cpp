#include "core/texture.h"

#include "../test_cfg.h"

using namespace glimpse;

void texture_test() {
  using namespace boost::ut;
  using namespace std;

  "solid_color"_test = [] {
    color c(0.8, 0.3, 0.2);
    solid_color texture(c);

    point3 point(1.0, 2.0, 3.0);
    color result = texture.value(0.5, 0.5, point);

    expect(that % result.x() == 0.8);
    expect(that % result.y() == 0.3);
    expect(that % result.z() == 0.2);
  };

  "checker_texture"_test = [] {
    color c1(1.0, 0.0, 0.0);  // Red
    color c2(0.0, 1.0, 0.0);  // Green
    double scale = 1.0;
    checker_texture texture(scale, c1, c2);

    // Points that should return "even" texture (red)
    point3 even_point1(0.5, 0.5, 0.5);  // 0+0+0 is even
    point3 even_point2(1.5, 1.5, 1.5);  // 1+1+1 is odd

    // Points that should return "odd" texture (green)
    point3 odd_point1(0.5, 0.5, 1.5);  // 0+0+1 is odd
    point3 odd_point2(1.5, 0.5, 0.5);  // 1+0+0 is odd

    expect(that % texture.value(0.0, 0.0, even_point1) == c1);
    expect(that % texture.value(0.0, 0.0, even_point2) == c2);
    expect(that % texture.value(0.0, 0.0, odd_point1) == c2);
    expect(that % texture.value(0.0, 0.0, odd_point2) == c2);
  };

  "noise_texture"_test = [] {
    double scale = 4.0;
    color base_color(0.5, 0.5, 0.5);
    noise_texture texture(scale, base_color);

    point3 p1(1.0, 2.0, 3.0);
    point3 p2(1.0, 2.0, 3.0);  // Same point
    point3 p3(4.0, 5.0, 6.0);  // Different point

    // Same point should produce the same color
    color c1 = texture.value(0.5, 0.5, p1);
    color c2 = texture.value(0.5, 0.5, p2);
    expect(that % c1 == c2);

    // Different point should (likely) produce different color
    color c3 = texture.value(0.5, 0.5, p3);
    expect(that % (c1.x() != c3.x() || c1.y() != c3.y() || c1.z() != c3.z()));

    // Check basic bounds - values should be reasonable for sine-based functions
    expect(c1.x() >= 0.0_d);
    expect(c1.y() >= 0.0_d);
    expect(c1.z() >= 0.0_d);
    expect(c1.x() <= 1.0_d);
    expect(c1.y() <= 1.0_d);
    expect(c1.z() <= 1.0_d);
  };
}