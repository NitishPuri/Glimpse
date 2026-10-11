#include "glimpse/util/color.h"

#include <cmath>

#include "glimpse/util/float_image.h"
#include "glimpse/util/image.h"

namespace glimpse {

double linearToSRGB(double x) {
  // Below the knee the curve is a straight line (a pure power curve would have infinite slope at 0, amplifying
  // noise in the darkest values); above it, a 1/2.4 power with offset, chosen so the two pieces meet smoothly.
  if (x <= 0.0031308) return 12.92 * x;
  return 1.055 * std::pow(x, 1.0 / 2.4) - 0.055;
}

Image encodeSRGB(const FloatImage& linear, double exposureStops) {
  const double scale = std::exp2(exposureStops);  // one stop = twice the light
  Image out(linear.width, linear.height);
  for (int y = 0; y < linear.height; ++y)
    for (int x = 0; x < linear.width; ++x) {
      const size_t i = linear.index(x, y);
      // setFloat clamps to [0, 1] and quantises to 8 bits.
      out.setFloat(x, y, static_cast<float>(linearToSRGB(scale * linear.rgb[i])),
                   static_cast<float>(linearToSRGB(scale * linear.rgb[i + 1])),
                   static_cast<float>(linearToSRGB(scale * linear.rgb[i + 2])));
    }
  return out;
}

}  // namespace glimpse
