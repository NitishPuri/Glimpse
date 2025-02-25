#pragma once

#include "vec3.h"

struct ImageColor {
  using uchar = unsigned char;
  ImageColor(uchar r, uchar g, uchar b) : rgb{r, g, b} {}
  ImageColor(const color &color) : rgb() {
    rgb[0] = static_cast<uchar>(256 * clamp(color.x(), 0, 0.999));
    rgb[1] = static_cast<uchar>(256 * clamp(color.y(), 0, 0.999));
    rgb[2] = static_cast<uchar>(256 * clamp(color.z(), 0, 0.999));
  }
  uchar rgb[3];
};
