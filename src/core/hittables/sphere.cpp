
#include "sphere.h"

bool sphere::hit(const ray &r, const interval &ray_t, hit_record &rec) const {
  vec3 oc = r.origin() - center;
  auto a = r.direction().length_squared();
  // TODO: Check this, why is -1 required, should the ray direction be reversed
  // in all places?
  auto half_b = dot(oc, -1 * r.direction());
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

bool sphere::bounding_box(double time0, double time1, aabb &output_box) const {
  output_box = aabb(center - vec3(radius, radius, radius),
                    center + vec3(radius, radius, radius));
  return true;
}