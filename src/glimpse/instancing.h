#pragma once

// Instancing: Translate and RotateY move a hittable by transforming the ray into its object space.
// Follows TNW "Instances"; pbrt-v4 §7.1 (transformed primitives).

#include "glimpse/interaction.h"

namespace glimpse {

class Translate : public Hittable {
 public:
  Translate(shared_ptr<Hittable> object, const Vec3& offset) : object(object), offset(offset) {
    bbox = object->boundingBox() + offset;
  }

  bool hit(const Ray& r, Interval rayT, HitRecord& rec) const override {
    // Move the ray backwards by the offset
    Ray offsetR(r.origin() - offset, r.direction(), r.time());

    // Determine whether an intersection exists along the offset ray (and if so, where)
    if (!object->hit(offsetR, rayT, rec)) return false;

    // Move the intersection point forwards by the offset
    rec.p += offset;

    return true;
  }

  AABB boundingBox() const override { return bbox; }

 private:
  shared_ptr<Hittable> object;
  Vec3 offset;
  AABB bbox;
};

class RotateY : public Hittable {
 public:
  RotateY(shared_ptr<Hittable> object, double angle) : object(object) {
    auto radians = math::degreesToRadians(angle);
    sinTheta = std::sin(radians);
    cosTheta = std::cos(radians);
    bbox = object->boundingBox();

    Point3 min(math::infinity, math::infinity, math::infinity);
    Point3 max(-math::infinity, -math::infinity, -math::infinity);

    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 2; j++) {
        for (int k = 0; k < 2; k++) {
          auto x = i * bbox.x.max + (1 - i) * bbox.x.min;
          auto y = j * bbox.y.max + (1 - j) * bbox.y.min;
          auto z = k * bbox.z.max + (1 - k) * bbox.z.min;

          auto newx = cosTheta * x + sinTheta * z;
          auto newz = -sinTheta * x + cosTheta * z;

          Vec3 tester(newx, y, newz);

          for (int c = 0; c < 3; c++) {
            min[c] = std::fmin(min[c], tester[c]);
            max[c] = std::fmax(max[c], tester[c]);
          }
        }
      }
    }

    bbox = AABB(min, max);
  }

  bool hit(const Ray& r, Interval rayT, HitRecord& rec) const override {
    // Transform the ray from world space to object space.

    auto origin = Point3((cosTheta * r.origin().x()) - (sinTheta * r.origin().z()), r.origin().y(),
                         (sinTheta * r.origin().x()) + (cosTheta * r.origin().z()));

    auto direction = Vec3((cosTheta * r.direction().x()) - (sinTheta * r.direction().z()), r.direction().y(),
                          (sinTheta * r.direction().x()) + (cosTheta * r.direction().z()));

    Ray rotatedR(origin, direction, r.time());

    // Determine whether an intersection exists in object space (and if so, where).

    if (!object->hit(rotatedR, rayT, rec)) return false;

    // Transform the intersection from object space back to world space.

    rec.p = Point3((cosTheta * rec.p.x()) + (sinTheta * rec.p.z()), rec.p.y(),
                   (-sinTheta * rec.p.x()) + (cosTheta * rec.p.z()));

    rec.normal = Vec3((cosTheta * rec.normal.x()) + (sinTheta * rec.normal.z()), rec.normal.y(),
                      (-sinTheta * rec.normal.x()) + (cosTheta * rec.normal.z()));

    return true;
  }

  AABB boundingBox() const override { return bbox; }

 private:
  shared_ptr<Hittable> object;
  double sinTheta;
  double cosTheta;
  AABB bbox;
};

}  // namespace glimpse
