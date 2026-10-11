#pragma once

#include "glimpse/interaction.h"
#include "glimpse/pdf.h"
#include "glimpse/textures.h"
#include "glimpse/util/sampling.h"

namespace glimpse {

class ScatterRecord {
 public:
  Color attenuation;
  shared_ptr<PDF> pdfPtr;
  bool skipPdf;
  Ray skipPdfRay;
};

class Material {
 public:
  virtual ~Material() = default;

  virtual Color emitted(const Ray& rIn, const HitRecord& rec, double u, double v, const Point3& p) const {
    return Color(0, 0, 0);
  }

  virtual bool scatter(const Ray& rIn, const HitRecord& rec, ScatterRecord& srec) const { return false; }

  virtual double scatteringPdf(const Ray& rIn, const HitRecord& rec, const Ray& scattered) const { return 0; }
};

class Lambertian : public Material {
 public:
  Lambertian(const Color& albedo) : tex(make_shared<SolidColor>(albedo)) {}
  Lambertian(shared_ptr<Texture> tex) : tex(tex) {}

  bool scatter(const Ray& rIn, const HitRecord& rec, ScatterRecord& srec) const override {
    srec.attenuation = tex->value(rec.u, rec.v, rec.p);
    srec.pdfPtr = make_shared<CosinePDF>(rec.normal);
    srec.skipPdf = false;
    return true;
  }

  double scatteringPdf(const Ray& rIn, const HitRecord& rec, const Ray& scattered) const override {
    auto cosTheta = dot(rec.normal, unitVector(scattered.direction()));
    return cosTheta < 0 ? 0 : cosTheta / math::pi;
  }

 private:
  shared_ptr<Texture> tex;
};

class Metal : public Material {
 public:
  Metal(const Color& albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

  bool scatter(const Ray& rIn, const HitRecord& rec, ScatterRecord& srec) const override {
    Vec3 reflected = reflect(rIn.direction(), rec.normal);
    reflected = unitVector(reflected) + (fuzz * randomUnitVector());

    srec.attenuation = albedo;
    srec.pdfPtr = nullptr;
    srec.skipPdf = true;
    srec.skipPdfRay = Ray(rec.p, reflected, rIn.time());

    return true;
  }

 private:
  Color albedo;
  double fuzz;
};

class Dielectric : public Material {
 public:
  Dielectric(double refractionIndex) : refractionIndex(refractionIndex) {}

  bool scatter(const Ray& rIn, const HitRecord& rec, ScatterRecord& srec) const override {
    srec.attenuation = Color(1.0, 1.0, 1.0);
    srec.pdfPtr = nullptr;
    srec.skipPdf = true;
    double ri = rec.frontFace ? (1.0 / refractionIndex) : refractionIndex;

    Vec3 unitDirection = unitVector(rIn.direction());
    double cosTheta = std::fmin(dot(-unitDirection, rec.normal), 1.0);
    double sinTheta = std::sqrt(1.0 - cosTheta * cosTheta);

    bool cannotRefract = ri * sinTheta > 1.0;
    Vec3 direction;

    if (cannotRefract || reflectance(cosTheta, ri) > randomDouble())
      direction = reflect(unitDirection, rec.normal);
    else
      direction = refract(unitDirection, rec.normal, ri);

    srec.skipPdfRay = Ray(rec.p, direction, rIn.time());
    return true;
  }

 private:
  // Refractive index in vacuum or air, or the ratio of the material's refractive index over
  // the refractive index of the enclosing media
  double refractionIndex;

  static double reflectance(double cosine, double refractionIndex) {
    // Use Schlick's approximation for reflectance.
    auto r0 = (1 - refractionIndex) / (1 + refractionIndex);
    r0 = r0 * r0;
    return r0 + (1 - r0) * std::pow((1 - cosine), 5);
  }
};

class DiffuseLight : public Material {
 public:
  DiffuseLight(shared_ptr<Texture> tex) : tex(tex) {}
  DiffuseLight(const Color& emit) : tex(make_shared<SolidColor>(emit)) {}

  Color emitted(const Ray& rIn, const HitRecord& rec, double u, double v, const Point3& p) const override {
    if (!rec.frontFace) return Color(0, 0, 0);
    return tex->value(u, v, p);
  }

 private:
  shared_ptr<Texture> tex;
};

class Isotropic : public Material {
 public:
  Isotropic(const Color& albedo) : tex(make_shared<SolidColor>(albedo)) {}
  Isotropic(shared_ptr<Texture> tex) : tex(tex) {}

  bool scatter(const Ray& rIn, const HitRecord& rec, ScatterRecord& srec) const override {
    srec.attenuation = tex->value(rec.u, rec.v, rec.p);
    srec.pdfPtr = make_shared<SpherePDF>();
    srec.skipPdf = false;
    return true;
  }

  double scatteringPdf(const Ray& rIn, const HitRecord& rec, const Ray& scattered) const override {
    return 1 / (4 * math::pi);
  }

 private:
  shared_ptr<Texture> tex;
};

}  // namespace glimpse
