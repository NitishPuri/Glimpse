#pragma once

#include "glimpse/util/rng.h"
#include "glimpse/util/vecmath.h"

namespace glimpse {

inline Vec3 randomUnitVector() {
  while (true) {
    auto p = Vec3::random(-1, 1);
    auto lensq = p.lengthSquared();
    if (1e-160 < lensq && lensq <= 1.0) return p / sqrt(lensq);
  }
}

inline Vec3 randomOnHemisphere(const Vec3& normal) {
  Vec3 onUnitSphere = randomUnitVector();
  if (dot(onUnitSphere, normal) > 0.0)  // In the same hemisphere as the normal
    return onUnitSphere;
  else
    return -onUnitSphere;
}

inline Vec3 randomInUnitDisk() {
  while (true) {
    auto p = Vec3(randomDouble(-1, 1), randomDouble(-1, 1), 0);
    if (p.lengthSquared() < 1) return p;
  }
}

// Generates a random point within a unit square centered at the origin
inline Vec3 sampleSquare() { return Vec3(randomDouble() - 0.5, randomDouble() - 0.5, 0); }

inline Vec3 randomCosineDirection() {
  auto r1 = randomDouble();
  auto r2 = randomDouble();

  auto phi = 2 * math::pi * r1;
  auto x = std::cos(phi) * std::sqrt(r2);
  auto y = std::sin(phi) * std::sqrt(r2);
  auto z = std::sqrt(1 - r2);

  return Vec3(x, y, z);
}

}  // namespace glimpse
