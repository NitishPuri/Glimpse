#include "glimpse/textures.h"

#include "glimpse/util/image.h"
#include "glimpse/util/interval.h"

namespace glimpse {

// TODO: Sampling floats directly from texture doesnt work,
// it kind of does, but there are it seems bands of missing data.
//  should figure out why that happens.
constexpr bool USE_FLOAT_PIXELS = false;

ImageTexture::ImageTexture(const char *filename) : image(std::make_shared<Image>(filename)) {}

Color ImageTexture::value(double u, double v, const Vec3 &p) const {
  // If we have no texture data, then return solid cyan as a debugging aid.
  // if (image == nullptr || image->height() <= 0) return color(0, 1, 1);
  if (!image || !image->isValid()) return Color(0, 1, 1);

  // Clamp input texture coordinates to [0,1] x [1,0]
  u = Interval(0, 1).clamp(u);
  v = 1.0 - Interval(0, 1).clamp(v);  // Flip V to image coordinates

  // Sampling from texture.
  // TODO:  Here we should consider at least linear filtering, but we are not doing it for now.
  // Should be a switch to visualize differences.
  // Or should we, ? since we are anyways doing random sampling!
  // Still should see effect of linear filtering here.
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
