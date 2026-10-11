#pragma once

#include <cmath>
#include <iostream>

#include "glimpse/util/math.h"
#include "glimpse/util/rng.h"

namespace glimpse {

class Vec3 {
 public:
  double e[3];

  Vec3() : e{0, 0, 0} {}
  Vec3(double e0, double e1, double e2) : e{e0, e1, e2} {}

  double x() const { return e[0]; }
  double y() const { return e[1]; }
  double z() const { return e[2]; }

  Vec3 operator-() const { return Vec3(-e[0], -e[1], -e[2]); }
  double operator[](int i) const { return e[i]; }
  double& operator[](int i) { return e[i]; }

  Vec3& operator+=(const Vec3& v) {
    e[0] += v.e[0];
    e[1] += v.e[1];
    e[2] += v.e[2];
    return *this;
  }

  Vec3& operator*=(double t) {
    e[0] *= t;
    e[1] *= t;
    e[2] *= t;
    return *this;
  }

  Vec3& operator/=(double t) { return *this *= 1 / t; }

  bool operator==(const Vec3& v) const { return e[0] == v.e[0] && e[1] == v.e[1] && e[2] == v.e[2]; }

  bool operator!=(const Vec3& v) const { return !(*this == v); }

  double length() const { return std::sqrt(lengthSquared()); }

  double lengthSquared() const { return e[0] * e[0] + e[1] * e[1] + e[2] * e[2]; }

  bool nearZero() const {
    // Return true if the vector is close to zero in all dimensions.
    auto s = 1e-8;
    return (std::fabs(e[0]) < s) && (std::fabs(e[1]) < s) && (std::fabs(e[2]) < s);
  }

  static Vec3 random() { return Vec3(randomDouble(), randomDouble(), randomDouble()); }

  static Vec3 random(double min, double max) {
    return Vec3(randomDouble(min, max), randomDouble(min, max), randomDouble(min, max));
  }

  friend std::ostream& operator<<(std::ostream& out, const Vec3& v);
};

// Define the ostream operator
inline std::ostream& operator<<(std::ostream& out, const Vec3& v) {
  return out << '(' << v.e[0] << ", " << v.e[1] << ", " << v.e[2] << ')';
}

// point3 is just an alias for vec3, but useful for geometric clarity in the code.
using Point3 = Vec3;
using Color = Vec3;

// Vector Utility Functions

inline Vec3 operator+(const Vec3& u, const Vec3& v) { return Vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]); }

inline Vec3 operator-(const Vec3& u, const Vec3& v) { return Vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]); }

inline Vec3 operator*(const Vec3& u, const Vec3& v) { return Vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]); }

inline Vec3 operator*(double t, const Vec3& v) { return Vec3(t * v.e[0], t * v.e[1], t * v.e[2]); }

inline Vec3 operator*(const Vec3& v, double t) { return t * v; }

inline Vec3 operator/(const Vec3& v, double t) { return (1 / t) * v; }

inline double dot(const Vec3& u, const Vec3& v) { return u.e[0] * v.e[0] + u.e[1] * v.e[1] + u.e[2] * v.e[2]; }

inline Vec3 cross(const Vec3& u, const Vec3& v) {
  return Vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1], u.e[2] * v.e[0] - u.e[0] * v.e[2], u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

inline Vec3 unitVector(const Vec3& v) { return v / v.length(); }

inline Vec3 sqrt(Vec3 v) { return Vec3(sqrt(v.e[0]), sqrt(v.e[1]), sqrt(v.e[2])); }

// Reflects vector v around normal vector n
// project v onto n and subtract the result twice to get the reflected vector
/*
   \ n /|
   v\|/ |
 ----|-------
     v\ |
       \|
*/
inline Vec3 reflect(const Vec3& v, const Vec3& n) { return v - 2 * dot(v, n) * n; }

// Refracts the vector `uv` through the surface with normal `n` using the
// ratio of indices of refraction `etaiOverEtat`
inline Vec3 refract(const Vec3& uv, const Vec3& n, double etaiOverEtat) {
  // Derived from Snell's law,
  auto cosTheta = std::fmin(dot(-uv, n), 1.0);
  Vec3 rOutPerp = etaiOverEtat * (uv + cosTheta * n);
  Vec3 rOutParallel = -std::sqrt(std::fabs(1.0 - rOutPerp.lengthSquared())) * n;
  return rOutPerp + rOutParallel;
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

class ONB {
 public:
  ONB(const Vec3& n) {
    axis[2] = unitVector(n);
    Vec3 a = (std::fabs(axis[2].x()) > 0.9) ? Vec3(0, 1, 0) : Vec3(1, 0, 0);
    axis[1] = unitVector(cross(axis[2], a));
    axis[0] = cross(axis[2], axis[1]);
  }

  const Vec3& u() const { return axis[0]; }
  const Vec3& v() const { return axis[1]; }
  const Vec3& w() const { return axis[2]; }

  Vec3 transform(const Vec3& v) const {
    // Transform from basis coordinates to local space.
    return (v[0] * axis[0]) + (v[1] * axis[1]) + (v[2] * axis[2]);
  }

 private:
  Vec3 axis[3];
};

}  // namespace glimpse
