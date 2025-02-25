#pragma once

#include "interval.h"
#include "ray.h"

class aabb {
 public:
  aabb() {}  // The default AABB is empty, since intervals are empty by default.

  aabb(const interval& x, const interval& y, const interval& z)
      : x(x), y(y), z(z) {
    pad_to_minimums();
  }

  aabb(const point3& a, const point3& b) {
    // Treat the two points a and b as extrema for the bounding box, so we don't
    // require a particular minimum/maximum coordinate order.

    x = (a[0] <= b[0]) ? interval(a[0], b[0]) : interval(b[0], a[0]);
    y = (a[1] <= b[1]) ? interval(a[1], b[1]) : interval(b[1], a[1]);
    z = (a[2] <= b[2]) ? interval(a[2], b[2]) : interval(b[2], a[2]);

    pad_to_minimums();
  }

  const interval& axis_interval(int n) const {
    if (n == 1) return y;
    if (n == 2) return z;
    return x;
  }

  aabb(const aabb& box0, const aabb& box1) {
    x = interval(box0.x, box1.x);
    y = interval(box0.y, box1.y);
    z = interval(box0.z, box1.z);
  }

  point3 min() const { return point3(x.min, y.min, z.min); }
  point3 max() const { return point3(x.max, y.max, z.max); }

  bool hit(const ray& r, interval ray_t) const;

  int longest_axis() const {
    // Returns the index of the longest axis of the bounding box.

    if (x.size() > y.size())
      return x.size() > z.size() ? 0 : 2;
    else
      return y.size() > z.size() ? 1 : 2;
  }

  static const aabb empty, universe;

  interval x, y, z;

  void pad_to_minimums() {
    // Adjust the AABB so that no side is narrower than some delta, padding if
    // necessary.

    double delta = 0.0001;
    if (x.size() < delta) x = x.expand(delta);
    if (y.size() < delta) y = y.expand(delta);
    if (z.size() < delta) z = z.expand(delta);
  }
};

inline aabb operator+(const aabb& bbox, const vec3& offset) {
  return aabb(bbox.x + offset.x(), bbox.y + offset.y(), bbox.z + offset.z());
}

inline aabb operator+(const vec3& offset, const aabb& bbox) {
  return bbox + offset;
}