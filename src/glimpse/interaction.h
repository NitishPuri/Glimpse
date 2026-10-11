#pragma once

#include "glimpse/util/aabb.h"

namespace glimpse {

class Material;

class HitRecord {
 public:
  Point3 p;
  Vec3 normal;
  shared_ptr<Material> mat;
  double t{};
  double u{};
  double v{};
  bool frontFace;

  void setFaceNormal(const Ray& r, const Vec3& outwardNormal) {
    // Sets the hit record normal vector.
    // NOTE: the parameter `outwardNormal` is assumed to have unit length.

    frontFace = dot(r.direction(), outwardNormal) < 0;
    normal = frontFace ? outwardNormal : -outwardNormal;
  }
};

class Hittable {
 public:
  virtual ~Hittable() = default;

  virtual bool hit(const Ray& r, Interval rayT, HitRecord& rec) const = 0;

  virtual AABB boundingBox() const = 0;

  virtual double pdfValue(const Point3& origin, const Vec3& direction) const { return 0.0; }

  virtual Vec3 random(const Point3& origin) const { return Vec3(1, 0, 0); }
};

}  // namespace glimpse
