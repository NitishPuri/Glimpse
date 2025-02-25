#include "aabb.h"

bool aabb::hit(const ray& r, interval ray_t) const {
  const point3& ray_orig = r.origin();
  const vec3& ray_dir = r.direction();

  for (int a = 0; a < 3; ++a) {
    const interval& ax = axis_interval(a);
    auto invD = 1.0f / ray_dir[a];

    auto t0 = (ax.min - r.origin()[a]) * invD;
    auto t1 = (ax.max - r.origin()[a]) * invD;

    if (t0 < t1) {
      if (t0 > ray_t.min) ray_t.min = t0;
      if (t1 < ray_t.max) ray_t.max = t1;
    } else {
      if (t1 > ray_t.min) ray_t.min = t1;
      if (t0 < ray_t.max) ray_t.max = t0;
    }
    if (ray_t.max <= ray_t.min) return false;
  }
  return true;
}
