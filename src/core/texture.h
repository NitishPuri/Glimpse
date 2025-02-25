#pragma once

#include "vec3.h"

class texture {
 public:
  virtual color value(double u, double v, const point3 &p) const = 0;
};

class solid_color : public texture {
 public:
  solid_color() {}
  solid_color(const color &c) : albedo(c) {}

  solid_color(double red, double green, double blue)
      : solid_color(color(red, green, blue)) {}

  color value(double u, double v, const point3 &p) const override {
    return albedo;
  }

 private:
  color albedo;
};

class checker_texture : public texture {
 public:
  checker_texture() {}
  checker_texture(double scale, shared_ptr<texture> _even,
                  shared_ptr<texture> _odd)
      : inv_scale(1.0 / scale), even(_even), odd(_odd) {}
  checker_texture(double scale, color c1, color c2)
      : checker_texture(scale, make_shared<solid_color>(c1),
                        (make_shared<solid_color>(c2))) {}

  color value(double u, double v, const point3 &p) const override {
    auto xInteger = int(std::floor(inv_scale * p.x()));
    auto yInteger = int(std::floor(inv_scale * p.y()));
    auto zInteger = int(std::floor(inv_scale * p.z()));

    bool isEven = (xInteger + yInteger + zInteger) % 2 == 0;

    return isEven ? even->value(u, v, p) : odd->value(u, v, p);
  }

 private:
  double inv_scale;
  shared_ptr<texture> odd;
  shared_ptr<texture> even;
};

class image_texture : public texture {
 public:
  const static int bytes_per_pixel = 3;

  image_texture() : data(nullptr), width(0), height(0), bytes_per_scanline(0) {}

  image_texture(const char *filename);

  ~image_texture() { delete data; }

  color value(double u, double v, const vec3 &p) const override;

 private:
  unsigned char *data;
  int width, height;
  int bytes_per_scanline;
};
