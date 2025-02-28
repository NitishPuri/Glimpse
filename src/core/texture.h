#pragma once

#include "perlin.h"
#include "vec3.h"

class rtw_image;

class texture {
 public:
  virtual ~texture() = default;

  virtual color value(double u, double v, const point3 &p) const = 0;
};

class solid_color : public texture {
 public:
  solid_color(const color &albedo) : albedo(albedo) {}

  solid_color(double red, double green, double blue) : solid_color(color(red, green, blue)) {}

  color value(double u, double v, const point3 &p) const override { return albedo; }

 private:
  color albedo;
};

class checker_texture : public texture {
 public:
  checker_texture(double scale, shared_ptr<texture> even, shared_ptr<texture> odd)
      : inv_scale(1.0 / scale), even(even), odd(odd) {}

  checker_texture(double scale, const color &c1, const color &c2)
      : checker_texture(scale, make_shared<solid_color>(c1), make_shared<solid_color>(c2)) {}

  color value(double u, double v, const point3 &p) const override {
    auto xInteger = int(std::floor(inv_scale * p.x()));
    auto yInteger = int(std::floor(inv_scale * p.y()));
    auto zInteger = int(std::floor(inv_scale * p.z()));

    bool isEven = (xInteger + yInteger + zInteger) % 2 == 0;

    return isEven ? even->value(u, v, p) : odd->value(u, v, p);
  }

 private:
  double inv_scale;
  shared_ptr<texture> even;
  shared_ptr<texture> odd;
};

class image_texture : public texture {
 public:
  image_texture(const char *filename);

  color value(double u, double v, const point3 &p) const override;

 private:
  std::shared_ptr<rtw_image> image;
};

class noise_texture : public texture {
 public:
  noise_texture(double scale, color color = color(.5, .5, .5)) : scale(scale), base_color(color) {}

  color value(double u, double v, const point3 &p) const override {
    // return base_color * (1 + std::sin(scale * p.z() + 10 * noise.turb(p, 7)));

    double dynamic_scale = scale * (1.2 * std::sin(1 * pi * u) + 0.3 * std::cos(3 * pi * v));
    return base_color * (1 + std::sin(dynamic_scale * p.z() + 10 * noise.turb(p, 7)));

    // double combined = scale * (p.x() * u + p.y() * v + p.z());
    // return base_color * (1 + std::sin(combined * p.z() + 10 * noise.turb(p, 7)));

    // double ripple = scale * std::sqrt(u * u + v * v);
    // return base_color * (1 + std::sin(ripple * p.z() + 10 * noise.turb(p, 7)));
  }

 private:
  perlin noise;
  double scale;
  color base_color;
};
