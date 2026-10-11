#pragma once

#include <cmath>
#include <iostream>

#include "glimpse/util/math.h"
#include "glimpse/util/rng.h"

namespace glimpse {

class vec3 {
 public:
  double e[3];

  vec3() : e{0, 0, 0} {}
  vec3(double e0, double e1, double e2) : e{e0, e1, e2} {}

  double x() const { return e[0]; }
  double y() const { return e[1]; }
  double z() const { return e[2]; }

  vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
  double operator[](int i) const { return e[i]; }
  double& operator[](int i) { return e[i]; }

  vec3& operator+=(const vec3& v) {
    e[0] += v.e[0];
    e[1] += v.e[1];
    e[2] += v.e[2];
    return *this;
  }

  vec3& operator*=(double t) {
    e[0] *= t;
    e[1] *= t;
    e[2] *= t;
    return *this;
  }

  vec3& operator/=(double t) { return *this *= 1 / t; }

  bool operator==(const vec3& v) const { return e[0] == v.e[0] && e[1] == v.e[1] && e[2] == v.e[2]; }

  bool operator!=(const vec3& v) const { return !(*this == v); }

  double length() const { return std::sqrt(length_squared()); }

  double length_squared() const { return e[0] * e[0] + e[1] * e[1] + e[2] * e[2]; }

  bool near_zero() const {
    // Return true if the vector is close to zero in all dimensions.
    auto s = 1e-8;
    return (std::fabs(e[0]) < s) && (std::fabs(e[1]) < s) && (std::fabs(e[2]) < s);
  }

  static vec3 random() { return vec3(random_double(), random_double(), random_double()); }

  static vec3 random(double min, double max) {
    return vec3(random_double(min, max), random_double(min, max), random_double(min, max));
  }

  friend std::ostream& operator<<(std::ostream& out, const vec3& v);
};

// Define the ostream operator
inline std::ostream& operator<<(std::ostream& out, const vec3& v) {
  return out << '(' << v.e[0] << ", " << v.e[1] << ", " << v.e[2] << ')';
}

// point3 is just an alias for vec3, but useful for geometric clarity in the code.
using point3 = vec3;
using color = vec3;

// Vector Utility Functions

inline vec3 operator+(const vec3& u, const vec3& v) { return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]); }

inline vec3 operator-(const vec3& u, const vec3& v) { return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]); }

inline vec3 operator*(const vec3& u, const vec3& v) { return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]); }

inline vec3 operator*(double t, const vec3& v) { return vec3(t * v.e[0], t * v.e[1], t * v.e[2]); }

inline vec3 operator*(const vec3& v, double t) { return t * v; }

inline vec3 operator/(const vec3& v, double t) { return (1 / t) * v; }

inline double dot(const vec3& u, const vec3& v) { return u.e[0] * v.e[0] + u.e[1] * v.e[1] + u.e[2] * v.e[2]; }

inline vec3 cross(const vec3& u, const vec3& v) {
  return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1], u.e[2] * v.e[0] - u.e[0] * v.e[2], u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

inline vec3 unit_vector(const vec3& v) { return v / v.length(); }

inline vec3 sqrt(vec3 v) { return vec3(sqrt(v.e[0]), sqrt(v.e[1]), sqrt(v.e[2])); }

// Reflects vector v around normal vector n
// project v onto n and subtract the result twice to get the reflected vector
/*
   \ n /|
   v\|/ |
 ----|-------
     v\ |
       \|
*/
inline vec3 reflect(const vec3& v, const vec3& n) { return v - 2 * dot(v, n) * n; }

// Refracts the vector `uv` through the surface with normal `n` using the
// ratio of indices of refraction `etai_over_etat`
inline vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat) {
  // Derived from Snell's law,
  auto cos_theta = std::fmin(dot(-uv, n), 1.0);
  vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
  vec3 r_out_parallel = -std::sqrt(std::fabs(1.0 - r_out_perp.length_squared())) * n;
  return r_out_perp + r_out_parallel;
}

// onb: orthonormal basis (from Ray Tracing: The Rest of Your Life).
//==============================================================================================
// Originally written in 2016 by Peter Shirley <ptrshrl@gmail.com>
//
// To the extent possible under law, the author(s) have dedicated all copyright and related and
// neighboring rights to this software to the public domain worldwide. This software is
// distributed without any warranty.
//
// You should have received a copy (see file COPYING.txt) of the CC0 Public Domain Dedication
// along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//==============================================================================================

class onb {
 public:
  onb(const vec3& n) {
    axis[2] = unit_vector(n);
    vec3 a = (std::fabs(axis[2].x()) > 0.9) ? vec3(0, 1, 0) : vec3(1, 0, 0);
    axis[1] = unit_vector(cross(axis[2], a));
    axis[0] = cross(axis[2], axis[1]);
  }

  const vec3& u() const { return axis[0]; }
  const vec3& v() const { return axis[1]; }
  const vec3& w() const { return axis[2]; }

  vec3 transform(const vec3& v) const {
    // Transform from basis coordinates to local space.
    return (v[0] * axis[0]) + (v[1] * axis[1]) + (v[2] * axis[2]);
  }

 private:
  vec3 axis[3];
};

}  // namespace glimpse
