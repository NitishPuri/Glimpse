#pragma once

#include "../material.h"
#include "../texture.h"
#include "hittable.h"

class constant_medium : public hittable {
 public:
  constant_medium(shared_ptr<hittable> b, double d, shared_ptr<texture> a)
      : boundary(b),
        neg_inv_density(-1 / d),
        phase_function(make_shared<isotropic>(a)) {}

  constant_medium(shared_ptr<hittable> b, double d, color c)
      : boundary(b),
        neg_inv_density(-1 / d),
        phase_function(make_shared<isotropic>(c)) {}

  virtual bool hit(const ray &r, const interval &ray_t,
                   hit_record &rec) const override;

  virtual bool bounding_box(double time0, double time1,
                            aabb &output_box) const override {
    return boundary->bounding_box(time0, time1, output_box);
  }

  aabb bounding_box() const override {
    // ??
    aabb output_box;
    if (boundary->bounding_box(0, 1, output_box)) {
      return output_box;
    } else {
      return aabb(point3(0, 0, 0), point3(0, 0, 0));
    }
  }

 public:
  shared_ptr<hittable> boundary;
  shared_ptr<material> phase_function;
  double neg_inv_density;
};
