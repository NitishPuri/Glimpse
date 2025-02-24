#pragma once

#include "interval.h"
#include "ray.h"

class aabb {
 public:
  aabb() {}
  aabb(const point3 &a, const point3 &b) { minimum = a, maximum = b; }

  point3 min() const { return minimum; }
  point3 max() const { return maximum; }

  bool hit(const ray &r, const interval &ray_t) const;

 private:
  point3 minimum;
  point3 maximum;
};

aabb surrounding_box(aabb box0, aabb box1);