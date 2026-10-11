#pragma once

#include "glimpse/perlin.h"
#include "glimpse/util/vecmath.h"

namespace glimpse {

class Image;

class Texture {
 public:
  virtual ~Texture() = default;

  virtual Color value(double u, double v, const Point3 &p) const = 0;
};

class SolidColor : public Texture {
 public:
  SolidColor(const Color &albedo) : albedo(albedo) {}

  SolidColor(double red, double green, double blue) : SolidColor(Color(red, green, blue)) {}

  Color value(double u, double v, const Point3 &p) const override { return albedo; }

 private:
  Color albedo;
};

class CheckerTexture : public Texture {
 public:
  CheckerTexture(double scale, shared_ptr<Texture> even, shared_ptr<Texture> odd)
      : invScale(1.0 / scale), even(even), odd(odd) {}

  CheckerTexture(double scale, const Color &c1, const Color &c2)
      : CheckerTexture(scale, make_shared<SolidColor>(c1), make_shared<SolidColor>(c2)) {}

  Color value(double u, double v, const Point3 &p) const override {
    auto xInteger = int(std::floor(invScale * p.x()));
    auto yInteger = int(std::floor(invScale * p.y()));
    auto zInteger = int(std::floor(invScale * p.z()));

    bool isEven = (xInteger + yInteger + zInteger) % 2 == 0;

    return isEven ? even->value(u, v, p) : odd->value(u, v, p);
  }

 private:
  double invScale;
  shared_ptr<Texture> even;
  shared_ptr<Texture> odd;
};

class ImageTexture : public Texture {
 public:
  ImageTexture(const char *filename);

  Color value(double u, double v, const Point3 &p) const override;

 private:
  std::shared_ptr<Image> image;
};

class NoiseTexture : public Texture {
 public:
  NoiseTexture(double scale, Color c = Color(.5, .5, .5)) : scale(scale), baseColor(c) {}

  Color value(double u, double v, const Point3 &p) const override {
    // return baseColor * (1 + std::sin(scale * p.z() + 10 * noise.turb(p, 7)));

    double dynamicScale = scale * (1.2 * std::sin(1 * math::pi * u) + 0.3 * std::cos(3 * math::pi * v));
    return baseColor * (1 + std::sin(dynamicScale * p.z() + 10 * noise.turb(p, 7)));

    // double combined = scale * (p.x() * u + p.y() * v + p.z());
    // return baseColor * (1 + std::sin(combined * p.z() + 10 * noise.turb(p, 7)));

    // double ripple = scale * std::sqrt(u * u + v * v);
    // return baseColor * (1 + std::sin(ripple * p.z() + 10 * noise.turb(p, 7)));
  }

 private:
  Perlin noise;
  double scale;
  Color baseColor;
};

}  // namespace glimpse
