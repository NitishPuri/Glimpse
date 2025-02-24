#pragma once

#include "hittable.h"

class moving_sphere : public hittable {
 public:
  moving_sphere() {}
  moving_sphere(point3 cen0, point3 cen1, double _time0, double _time1,
                double r, shared_ptr<material> m)
      : center0{cen0},
        center1{cen1},
        time0{_time0},
        time1{_time1},
        radius{r},
        mat_ptr{m} {}

  bool hit(const ray &r, const interval &ray_t, hit_record &rec) const override;
  bool bounding_box(double time0, double time1,
                    aabb &output_box) const override;

  point3 center(double time) const {
    return center0 + (center1 - center0) * ((time - time0) / (time1 - time0));
  }

 public:
  point3 center0, center1;
  double time0, time1;
  double radius;
  shared_ptr<material> mat_ptr;
};
