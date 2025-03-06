#include "texture.h"

#include "interval.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "rtw_stb_image.h"

// TODO: Sampling floats directly from texture doesnt work,
// it kind of does, but there are it seems bands of missing data.
//  should figure out why that happens.
constexpr bool USE_FLOAT_PIXELS = false;

image_texture::image_texture(const char *filename) : image(std::make_shared<rtw_image>(filename)) {}

color image_texture::value(double u, double v, const vec3 &p) const {
  // If we have no texture data, then return solid cyan as a debugging aid.
  if (image == nullptr || image->height() <= 0) return color(0, 1, 1);

  // Clamp input texture coordinates to [0,1] x [1,0]
  u = interval(0, 1).clamp(u);
  v = 1.0 - interval(0, 1).clamp(v);  // Flip V to image coordinates

  // Sampling from texture.
  // TODO:  Here we should consider at least linear filtering, but we are not doing it for now.
  // Should be a switch to visualize differences.
  // Or should we, ? since we are anyways doing random sampling!
  // Still should see effect of linear filtering here.
  auto i = std::min(static_cast<int>(u * image->width()), image->width() - 1);
  auto j = std::min(static_cast<int>(v * image->height()), image->height() - 1);

  if (USE_FLOAT_PIXELS) {
    auto pixel = image->pixel_data_f(i, j);
    double r = std::max(0.0, std::min(1.0, static_cast<double>(pixel[0])));
    double g = std::max(0.0, std::min(1.0, static_cast<double>(pixel[1])));
    double b = std::max(0.0, std::min(1.0, static_cast<double>(pixel[2])));
    return color(r, g, b);
  }

  auto pixel = image->pixel_data(i, j);
  const auto color_scale = 1.0 / 255.0;
  return color(color_scale * pixel[0], color_scale * pixel[1], color_scale * pixel[2]);
}
