#pragma once

#include "glimpse/util/interval.h"
#include "glimpse/ray.h"

namespace glimpse {
class AABB {
 public:
  Interval x, y, z;

  AABB() {}  // The default AABB is empty, since intervals are empty by default.

  AABB(const Interval& x, const Interval& y, const Interval& z) : x(x), y(y), z(z) { padToMinimums(); }

  AABB(const Point3& a, const Point3& b) {
    // Treat the two points a and b as extrema for the bounding box, so we don't require a
    // particular minimum/maximum coordinate order.

    x = (a[0] <= b[0]) ? Interval(a[0], b[0]) : Interval(b[0], a[0]);
    y = (a[1] <= b[1]) ? Interval(a[1], b[1]) : Interval(b[1], a[1]);
    z = (a[2] <= b[2]) ? Interval(a[2], b[2]) : Interval(b[2], a[2]);

    padToMinimums();
  }

  AABB(const AABB& box0, const AABB& box1) {
    x = Interval(box0.x, box1.x);
    y = Interval(box0.y, box1.y);
    z = Interval(box0.z, box1.z);
  }

  const Interval& axisInterval(int n) const {
    if (n == 1) return y;
    if (n == 2) return z;
    return x;
  }

  bool hit(const Ray& r, Interval rayT) const;

  int longestAxis() const {
    // Returns the index of the longest axis of the bounding box.

    if (x.size() > y.size())
      return x.size() > z.size() ? 0 : 2;
    else
      return y.size() > z.size() ? 1 : 2;
  }

  static const AABB empty, universe;

 private:
  void padToMinimums() {
    // Adjust the AABB so that no side is narrower than some delta, padding if necessary.

    double delta = 0.0001;
    if (x.size() < delta) x = x.expand(delta);
    if (y.size() < delta) y = y.expand(delta);
    if (z.size() < delta) z = z.expand(delta);
  }
};

// C++17 inline definitions (formerly in glimpse.cpp).
inline const AABB AABB::empty = AABB(Interval::empty, Interval::empty, Interval::empty);
inline const AABB AABB::universe = AABB(Interval::universe, Interval::universe, Interval::universe);

inline AABB operator+(const AABB& bbox, const Vec3& offset) {
  return AABB(bbox.x + offset.x(), bbox.y + offset.y(), bbox.z + offset.z());
}

inline AABB operator+(const Vec3& offset, const AABB& bbox) { return bbox + offset; }

}  // namespace glimpse