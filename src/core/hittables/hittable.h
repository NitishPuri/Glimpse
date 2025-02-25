#pragma once

// #include "aabb.h"
#include "../aabb.h"
#include "../common.h"

class material;

struct hit_record {
  point3 p{};
  vec3 normal{};
  double t{};
  double u{};
  double v{};
  bool front_face{};
  shared_ptr<material> mat_ptr;

  inline void set_face_normal(const ray& r, const vec3& outward_normal) {
  front_face = dot(r.direction(), outward_normal) < 0;
    normal = front_face ? outward_normal : -outward_normal;
  }
};

class hittable {
 public:
  virtual ~hittable() = default;
  virtual bool hit(const ray& r, const interval& ray_t,
                   hit_record& rec) const = 0;
  virtual bool bounding_box(double time0, double time1,
                            aabb& output_box) const = 0;
  virtual aabb bounding_box() const = 0;
};

class translate : public hittable {
 public:
  translate(shared_ptr<hittable> p, const vec3& displacement)
      : ptr(p), offset(displacement) {}

  virtual bool hit(const ray& r, const interval& ray_t,
                   hit_record& rec) const override;

  virtual bool bounding_box(double time0, double time1,
                            aabb& output_box) const override;
  aabb bounding_box() const override { return ptr->bounding_box(); }

 public:
  shared_ptr<hittable> ptr;
  vec3 offset;
};

class rotate_y : public hittable {
 public:
  rotate_y(shared_ptr<hittable> p, double angle);

  virtual bool hit(const ray& r, const interval& ray_t,
                   hit_record& rec) const override;

  virtual bool bounding_box(double time0, double time1,
                            aabb& output_box) const override {
    output_box = bbox;
    return hasbox;
  }
  aabb bounding_box() const override { return ptr->bounding_box(); }

 public:
  shared_ptr<hittable> ptr;
  double sin_theta;
  double cos_theta;
  bool hasbox;
  aabb bbox;
};
