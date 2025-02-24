#include "moving_sphere.h"

bool moving_sphere::hit(const ray &r, const interval &ray_t,
                        hit_record &rec) const {
  vec3 oc = r.origin() - center(r.time());
  auto a = r.direction().length_squared();
  auto half_b = dot(oc, r.direction());
  auto c = oc.length_squared() - radius * radius;

  auto discriminant = half_b * half_b - a * c;
  if (discriminant < 0) {
    return false;
  }
  auto sqrtd = sqrt(discriminant);

  // Find the nearest root that lies in the acceptable range.
  auto root = (-half_b - sqrtd) / a;
  if (root < ray_t.min || root > ray_t.max) {
    root = (-half_b + sqrtd) / a;
    if (root < ray_t.min || ray_t.max < root) return false;
  }

  rec.t = root;
  rec.p = r.at(rec.t);
  auto outward_normal = (rec.p - center(r.time())) / radius;
  rec.set_face_normal(r, outward_normal);
  rec.mat_ptr = mat_ptr;

  return true;
}

bool moving_sphere::bounding_box(double _time0, double _time1,
                                 aabb &output_box) const {
  aabb box0(center(_time0) - vec3(radius, radius, radius),
            center(_time0) + vec3(radius, radius, radius));
  aabb box1(center(_time1) - vec3(radius, radius, radius),
            center(_time1) + vec3(radius, radius, radius));
  output_box = surrounding_box(box0, box1);
  return true;
}