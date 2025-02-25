#pragma once

// #include "hittable.h"
#include "hittable_list.h"

class box : public hittable {
 public:
  box() {}
  box(const point3 &p0, const point3 &p1, shared_ptr<material> ptr);

  bool hit(const ray &r, const interval &ray_t, hit_record &rec) const override;

  bool bounding_box(double time0, double time1,
                    aabb &output_box) const override;
  aabb bounding_box() const override { return bbox; }

 public:
  point3 box_min;
  point3 box_max;
  aabb bbox;
  hittable_list sides;
};
