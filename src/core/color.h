#pragma once

#include "vec3.h"
#include "interval.h"

inline double linear_to_gamma(double linear_component) {
  if (linear_component > 0) return std::sqrt(linear_component);

  return 0;
}

struct ImageColor {
  using uchar = unsigned char;
  ImageColor(uchar r, uchar g, uchar b) : rgb{r, g, b} {}
  ImageColor(const color &color) : rgb() {
    auto r = color.x();
    auto g = color.y();
    auto b = color.z();

    // Replace NaN components with zero.
    if (r != r) r = 0.0;
    if (g != g) g = 0.0;
    if (b != b) b = 0.0;

    // Apply a linear to gamma transform for gamma 2
    r = linear_to_gamma(r);
    g = linear_to_gamma(g);
    b = linear_to_gamma(b);

    static const interval intensity(0.000, 0.999);
    rgb[0] = static_cast<uchar>(256 * intensity.clamp(r));
    rgb[1] = static_cast<uchar>(256 * intensity.clamp(g));
    rgb[2] = static_cast<uchar>(256 * intensity.clamp(b));
  }
  uchar rgb[3];
};
