#include "texture.h"

#include "image.h"
#include "interval.h"

using namespace glimpse;

// TODO: Sampling floats directly from texture doesnt work,
// it kind of does, but there are it seems bands of missing data.
//  should figure out why that happens.
constexpr bool USE_FLOAT_PIXELS = false;

image_texture::image_texture(const char *filename) : m_image(std::make_shared<Image>(filename)) {}

color image_texture::value(double u, double v, const vec3 &p) const {
  // If we have no texture data, then return solid cyan as a debugging aid.
  // if (m_image == nullptr || m_image->height() <= 0) return color(0, 1, 1);
  if (!m_image || !m_image->is_valid()) return color(0, 1, 1);

  // Clamp input texture coordinates to [0,1] x [1,0]
  u = interval(0, 1).clamp(u);
  v = 1.0 - interval(0, 1).clamp(v);  // Flip V to image coordinates

  // Sampling from texture.
  // TODO:  Here we should consider at least linear filtering, but we are not doing it for now.
  // Should be a switch to visualize differences.
  // Or should we, ? since we are anyways doing random sampling!
  // Still should see effect of linear filtering here.
  auto i = std::min(static_cast<int>(u * m_image->width), m_image->width - 1);
  auto j = std::min(static_cast<int>(v * m_image->height), m_image->height - 1);

  if (USE_FLOAT_PIXELS) {
    return m_image->get_float(i, j);
  }

  auto pixel = m_image->get(i, j);
  const auto color_scale = 1.0 / 255.0;
  return color(color_scale * pixel.rgb[0], color_scale * pixel.rgb[1], color_scale * pixel.rgb[2]);
}
