#pragma once

#include "glimpse.h"

class aabb {
 public:
  aabb() {}
  aabb(const point3 &a, const point3 &b) { minimum = a, maximum = b; }

  point3 min() const { return minimum; }
  point3 max() const { return maximum; }

  //   bool hit(const ray &r, interval ray_t) const;

  bool hit(const ray &r, interval ray_t) const {
    for (int a = 0; a < 3; ++a) {
      auto invD = 1.0f / r.direction()[a];
      auto t0 = (minimum[a] - r.origin()[a]) * invD;
      auto t1 = (maximum[a] - r.origin()[a]) * invD;
      if (invD < 0.0f) std::swap(t0, t1);
      ray_t.min = t0 > ray_t.min ? t0 : ray_t.min;
      ray_t.max = t1 < ray_t.max ? t1 : ray_t.max;
      if (ray_t.max < ray_t.min) return false;
    }
    return true;
  }

 private:
  point3 minimum;
  point3 maximum;
};

aabb surrounding_box(aabb box0, aabb box1) {
  point3 small(fmin(box0.min().x(), box1.min().x()),
               fmin(box0.min().y(), box1.min().y()),
               fmin(box0.min().z(), box1.min().z()));

  point3 big(fmax(box0.max().x(), box1.max().x()),
             fmax(box0.max().y(), box1.max().y()),
             fmax(box0.max().z(), box1.max().z()));

  return aabb(small, big);
}