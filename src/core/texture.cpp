#include "texture.h"

#include "interval.h"
#include "rtw_stb_image.h"

image_texture::image_texture(const char *filename)
    : image(std::make_shared<rtw_image>(filename)) {}

color image_texture::value(double u, double v, const vec3 &p) const {
  // If we have no texture data, then return solid cyan as a debugging aid.
  if (image == nullptr || image->height() <= 0) return color(0, 1, 1);

  // Clamp input texture coordinates to [0,1] x [1,0]
  u = interval(0, 1).clamp(u);
  v = 1.0 - interval(0, 1).clamp(v);  // Flip V to image coordinates

  auto i = static_cast<int>(u * image->width());
  auto j = static_cast<int>(v * image->height());

  auto pixel = image->pixel_data(i, j);

  const auto color_scale = 1.0 / 255.0;
  return color(color_scale * pixel[0], color_scale * pixel[1],
               color_scale * pixel[2]);
}
