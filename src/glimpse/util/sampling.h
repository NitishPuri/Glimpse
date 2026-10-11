#pragma once

#include "glimpse/util/rng.h"
#include "glimpse/util/vecmath.h"

namespace glimpse {

inline vec3 random_unit_vector() {
  while (true) {
    auto p = vec3::random(-1, 1);
    auto lensq = p.length_squared();
    if (1e-160 < lensq && lensq <= 1.0) return p / sqrt(lensq);
  }
}

inline vec3 random_on_hemisphere(const vec3& normal) {
  vec3 on_unit_sphere = random_unit_vector();
  if (dot(on_unit_sphere, normal) > 0.0)  // In the same hemisphere as the normal
    return on_unit_sphere;
  else
    return -on_unit_sphere;
}

inline vec3 random_in_unit_disk() {
  while (true) {
    auto p = vec3(random_double(-1, 1), random_double(-1, 1), 0);
    if (p.length_squared() < 1) return p;
  }
}

// Generates a random point within a unit square centered at the origin
inline vec3 sample_square() { return vec3(random_double() - 0.5, random_double() - 0.5, 0); }

inline vec3 random_cosine_direction() {
  auto r1 = random_double();
  auto r2 = random_double();

  auto phi = 2 * math::pi * r1;
  auto x = std::cos(phi) * std::sqrt(r2);
  auto y = std::sin(phi) * std::sqrt(r2);
  auto z = std::sqrt(1 - r2);

  return vec3(x, y, z);
}

}  // namespace glimpse
