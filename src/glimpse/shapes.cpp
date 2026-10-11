#include "glimpse/shapes.h"

namespace glimpse {

bool MovingSphere::hit(const Ray &r, Interval rayT, HitRecord &rec) const {
  auto currentCenter = center.at(r.time());
  Vec3 oc = currentCenter - r.origin();
  auto a = r.direction().lengthSquared();
  auto halfB = dot(oc, r.direction());
  auto c = oc.lengthSquared() - radius * radius;

  auto discriminant = halfB * halfB - a * c;
  if (discriminant < 0) {
    return false;
  }
  auto sqrtd = sqrt(discriminant);

  // Find the nearest root that lies in the acceptable range.
  auto root = (halfB - sqrtd) / a;
  if (root < rayT.min || root > rayT.max) {
    root = (halfB + sqrtd) / a;
    if (root < rayT.min || rayT.max < root) return false;
  }

  rec.t = root;
  rec.p = r.at(rec.t);
  auto outwardNormal = (rec.p - currentCenter) / radius;
  rec.setFaceNormal(r, outwardNormal);
  rec.mat = matPtr;

  return true;
}

}  // namespace glimpse
