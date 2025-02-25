#pragma once

#include "hittable.h"

class moving_sphere : public hittable {
 public:
  moving_sphere() {}
  moving_sphere(point3 cen0, point3 cen1, double r, shared_ptr<material> m)
      : center{cen0, cen1 - cen0}, radius{r}, mat_ptr{m} {}

  bool hit(const ray &r, const interval &ray_t, hit_record &rec) const override;
  bool bounding_box(double time0, double time1,
                    aabb &output_box) const override;

 public:
  ray center;
  double time0, time1;
  double radius;
  shared_ptr<material> mat_ptr;
};
