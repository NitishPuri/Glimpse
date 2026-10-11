#pragma once

// Ray: origin + t * direction, plus a time for motion blur.
// Follows RTIOW "Rays, a Simple Camera, and Background" and TNW "Motion Blur"; pbrt-v4 §3.6.

#include "glimpse/util/vecmath.h"

namespace glimpse {

class Ray {
 public:
  Ray() {}

  Ray(const Point3& origin, const Vec3& direction, double time) : orig(origin), dir(direction), tm(time) {}

  Ray(const Point3& origin, const Vec3& direction) : Ray(origin, direction, 0) {}

  const Point3& origin() const { return orig; }
  const Vec3& direction() const { return dir; }

  double time() const { return tm; }

  Point3 at(double t) const { return orig + t * dir; }

 private:
  Point3 orig;
  Vec3 dir;
  double tm{};
};

}  // namespace glimpse