#include "aabb.h"

bool aabb::hit(const ray &r, const interval &ray_t) const {
  auto t_min = ray_t.min;
  auto t_max = ray_t.max;
  for (int a = 0; a < 3; ++a) {
    auto invD = 1.0f / r.direction()[a];
    auto t0 = (minimum[a] - r.origin()[a]) * invD;
    auto t1 = (maximum[a] - r.origin()[a]) * invD;
    if (invD < 0.0f) std::swap(t0, t1);
    t_min = t0 > t_min ? t0 : t_min;
    t_max = t1 < t_max ? t1 : t_max;
    if (t_max < t_min) return false;
  }
  return true;
}

aabb surrounding_box(aabb box0, aabb box1) {
  point3 small(fmin(box0.min().x(), box1.min().x()),
               fmin(box0.min().y(), box1.min().y()),
               fmin(box0.min().z(), box1.min().z()));

  point3 big(fmax(box0.max().x(), box1.max().x()),
             fmax(box0.max().y(), box1.max().y()),
             fmax(box0.max().z(), box1.max().z()));

  return aabb(small, big);
}