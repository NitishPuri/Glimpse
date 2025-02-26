#pragma once

#include "hittables/hittable.h"
#include "onb.h"
#include "ray.h"
#include "texture.h"

class material {
 public:
  // TODO: Add rand_state to seed randomness for reproducability.
  //  This can be configured in other places that use randomness as well.
  virtual bool scatter(const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered, double &pdf) const {
    return false;
  }

  // Emitted light - zero by default
  // virtual color emitted(double u, double v, const point3 &p) const { return color(0, 0, 0); }
  virtual color emitted(const ray &r_in, const hit_record &rec, double u, double v, const point3 &p) const {
    return color(0, 0, 0);
  }

  virtual double scattering_pdf(const ray &r_in, const hit_record &rec, const ray &scattered) const { return 0; }
};

class lambertian : public material {
 public:
  lambertian(const color &a) : albedo(make_shared<solid_color>(a)) {}
  lambertian(shared_ptr<texture> a) : albedo(a) {}

  virtual bool scatter(const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered,
                       double &pdf) const override {
    // auto scatter_direction = rec.normal + random_unit_vector();  // labertian
    // auto scatter_direction = random_in_hemisphere(rec.normal); // hemispherical
    onb uvw(rec.normal);
    vec3 scatter_direction = uvw.transform(random_cosine_direction());

    if (scatter_direction.near_zero())  // catch degenerate scatter direction
      scatter_direction = rec.normal;

    scattered = ray(rec.p, scatter_direction, r_in.time());
    attenuation = albedo->value(rec.u, rec.v, rec.p);

    pdf = dot(uvw.w(), scattered.direction()) / pi;
    return true;
  }

  double scattering_pdf(const ray &r_in, const hit_record &rec, const ray &scattered) const override {
    // auto cos_theta = dot(rec.normal, unit_vector(scattered.direction()));
    // return cos_theta < 0 ? 0 : cos_theta / pi;

    return 1 / (2 * pi);
  }

 public:
  shared_ptr<texture> albedo;
};

class metal : public material {
 public:
  metal(const color &a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

  virtual bool scatter(const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered,
                       double &pdf) const override {
    vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
    scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere(), r_in.time());
    attenuation = albedo;
    return (dot(scattered.direction(), rec.normal) > 0);
  }

 public:
  color albedo;
  double fuzz;
};

class dielectric : public material {
 public:
  dielectric(double index_of_refraction) : refraction_index(index_of_refraction) {}

  virtual bool scatter(const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered,
                       double &pdf) const override {
    attenuation = color(1.0, 1.0, 1.0);
    double refraction_ratio = rec.front_face ? (1.0 / refraction_index) : refraction_index;
    // double refraction_ratio =
    //     (!rec.front_face) ? (1.0 / refraction_index) : refraction_index;

    vec3 unit_direction = unit_vector(r_in.direction());
    double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
    double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

    // Snell's law again,
    // assuming only interaction between vacuum/air and material
    // refraction_index captures whether we are going in the material or out.
    // according to snells law sin(theta_r) = sin (theta_i) * n1 / n2
    // if this term is more than one, refraction cannot occur.
    bool cannot_refract = refraction_ratio * sin_theta > 1.0;
    vec3 direction;

    if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double()) {
      direction = reflect(unit_direction, rec.normal);
    } else {
      direction = refract(unit_direction, rec.normal, refraction_ratio);
    }

    scattered = ray(rec.p, direction, r_in.time());
    return true;
  }

 public:
  // Refractive index in vacuum or air, or the ratio of the material's
  // refractive index over
  // the refractive index of the enclosing media
  double refraction_index;

 private:
  // Glass has reflectivity that varies with aangle, a window looked at a steep
  // angle becomes a mirror. There is a big ugle equation for that, but
  // everybody uses a cheap and accurate apprimation by Christophe Schlick
  static double reflectance(double cosine, double ref_idx) {
    // Use Schlick's approximation for reflactance
    auto r0 = (1 - ref_idx) / (1 + ref_idx);
    r0 = r0 * r0;
    return r0 + (1 - r0) * pow((1 - cosine), 5);
  }
};

class diffuse_light : public material {
 public:
  diffuse_light(shared_ptr<texture> a) : emit(a) {}
  diffuse_light(color c) : emit(make_shared<solid_color>(c)) {}

  bool scatter(const ray &ray_in, const hit_record &rec, color &attenuation, ray &scattered,
               double &pdf) const override {
    return false;
  }

  // color emitted(double u, double v, const point3 &p) const override { return emit->value(u, v, p); }
  color emitted(const ray &r_in, const hit_record &rec, double u, double v, const point3 &p) const override {
    if(!rec.front_face) return color(0, 0, 0);
    return emit->value(u, v, p);
  }

 private:
  shared_ptr<texture> emit;
};

class isotropic : public material {
 public:
  isotropic(color c) : albedo(make_shared<solid_color>(c)) {}
  isotropic(shared_ptr<texture> a) : albedo(a) {}

  virtual bool scatter(const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered,
                       double &pdf) const override {
    scattered = ray(rec.p, random_in_unit_sphere(), r_in.time());
    attenuation = albedo->value(rec.u, rec.v, rec.p);
    pdf = 1.0 / (4 * pi);
    return true;
  }

  double scattering_pdf(const ray &r_in, const hit_record &rec, const ray &scattered) const override {
    return 1 / (4 * pi);
  }

 public:
  shared_ptr<texture> albedo;
};