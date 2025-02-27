// #include "aabb.h"

// bool aabb::hit(const ray& r, interval ray_t) const {
//   const point3& ray_orig = r.origin();
//   const vec3& ray_dir = r.direction();

//   for (int axis = 0; axis < 3; axis++) {
//     const interval& ax = axis_interval(axis);
//     const double adinv = 1.0 / ray_dir[axis];

//     auto t0 = (ax.min - ray_orig[axis]) * adinv;
//     auto t1 = (ax.max - ray_orig[axis]) * adinv;

//     if (t0 < t1) {
//       if (t0 > ray_t.min) ray_t.min = t0;
//       if (t1 < ray_t.max) ray_t.max = t1;
//     } else {
//       if (t1 > ray_t.min) ray_t.min = t1;
//       if (t0 < ray_t.max) ray_t.max = t0;
//     }

//     if (ray_t.max <= ray_t.min) return false;
//   }
//   return true;
// }