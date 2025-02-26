#pragma once

#include "hittable.h"

class moving_sphere : public hittable {
 public:
  moving_sphere() {}
  moving_sphere(point3 cen0, point3 cen1, double r, shared_ptr<material> m)
      : center{cen0, cen1 - cen0}, radius{r}, mat_ptr{m} {
    auto rvec = vec3(radius, radius, radius);
    aabb box1(center.at(0) - rvec, center.at(0) + rvec);
    aabb box2(center.at(1) - rvec, center.at(1) + rvec);
    bbox = aabb(box1, box2);
  }

  bool hit(const ray &r, const interval &ray_t, hit_record &rec) const override;

  aabb bounding_box() const override { return bbox; }

 public:
  ray center;
  double time0{}, time1{};
  double radius{};
  aabb bbox;
  shared_ptr<material> mat_ptr;
};
