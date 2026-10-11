#include "glimpse/util/aabb.h"

namespace glimpse {

bool AABB::hit(const Ray& r, Interval rayT) const {
  const Point3& rayOrig = r.origin();
  const Vec3& rayDir = r.direction();

  for (int axis = 0; axis < 3; axis++) {
    const Interval& ax = axisInterval(axis);
    const double adinv = 1.0 / rayDir[axis];

    auto t0 = (ax.min - rayOrig[axis]) * adinv;
    auto t1 = (ax.max - rayOrig[axis]) * adinv;

    if (t0 < t1) {
      if (t0 > rayT.min) rayT.min = t0;
      if (t1 < rayT.max) rayT.max = t1;
    } else {
      if (t1 > rayT.min) rayT.min = t1;
      if (t0 < rayT.max) rayT.max = t0;
    }

    if (rayT.max <= rayT.min) return false;
  }
  return true;
}

}  // namespace glimpse
