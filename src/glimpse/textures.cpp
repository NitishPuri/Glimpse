// ImageTexture: loads an image file and looks up texels with (u, v) in [0,1]².

#include "glimpse/textures.h"

#include "glimpse/util/image.h"
#include "glimpse/util/interval.h"

namespace glimpse {

// Known issue (backlog): sampling the float pixels directly shows bands of missing data, so textures read the
// 8-bit pixels for now.
constexpr bool USE_FLOAT_PIXELS = false;

ImageTexture::ImageTexture(const char *filename) : image(std::make_shared<Image>(filename)) {}

Color ImageTexture::value(double u, double v, const Vec3 &p) const {
  // If we have no texture data, then return solid cyan as a debugging aid.
  if (!image || !image->isValid()) return Color(0, 1, 1);

  // Clamp input texture coordinates to [0,1] x [1,0]
  u = Interval(0, 1).clamp(u);
  v = 1.0 - Interval(0, 1).clamp(v);  // Flip V to image coordinates

  // Sampling from texture.
  // Nearest-neighbour lookup; filtering (pbrt §10.4) is in the backlog.
  auto i = std::min(static_cast<int>(u * image->width), image->width - 1);
  auto j = std::min(static_cast<int>(v * image->height), image->height - 1);

  if (USE_FLOAT_PIXELS) {
    return image->getFloat(i, j);
  }

  auto pixel = image->get(i, j);
  const auto colorScale = 1.0 / 255.0;
  return Color(colorScale * pixel.rgb[0], colorScale * pixel.rgb[1], colorScale * pixel.rgb[2]);
}

}  // namespace glimpse
