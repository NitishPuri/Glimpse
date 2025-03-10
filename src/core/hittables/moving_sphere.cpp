#include "moving_sphere.h"

using namespace glimpse;

bool moving_sphere::hit(const ray &r, interval ray_t, hit_record &rec) const {
  auto current_center = center.at(r.time());
  vec3 oc = current_center - r.origin();
  auto a = r.direction().length_squared();
  auto half_b = dot(oc, r.direction());
  auto c = oc.length_squared() - radius * radius;

  auto discriminant = half_b * half_b - a * c;
  if (discriminant < 0) {
    return false;
  }
  auto sqrtd = sqrt(discriminant);

  // Find the nearest root that lies in the acceptable range.
  auto root = (half_b - sqrtd) / a;
  if (root < ray_t.min || root > ray_t.max) {
    root = (half_b + sqrtd) / a;
    if (root < ray_t.min || ray_t.max < root) return false;
  }

  rec.t = root;
  rec.p = r.at(rec.t);
  auto outward_normal = (rec.p - current_center) / radius;
  rec.set_face_normal(r, outward_normal);
  rec.mat = mat_ptr;

  return true;
}
