#include "sphere.h"

bool sphere::hit(const ray &r, const interval &ray_t, hit_record &rec) const {
  vec3 oc = center - r.origin();
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
  if (!ray_t.surrounds(root)) {
    root = (half_b + sqrtd) / a;
    if (!ray_t.surrounds(root)) return false;
  }

  rec.t = root;
  rec.p = r.at(rec.t);
  vec3 outward_normal = (rec.p - center) / radius;
  rec.set_face_normal(r, outward_normal);
  get_sphere_uv(outward_normal, rec.u, rec.v);
  rec.mat_ptr = mat_ptr;

  return true;
}
