#include "moving_sphere.h"

bool moving_sphere::hit(const ray &r, const interval &ray_t,
                        hit_record &rec) const {
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
  rec.mat_ptr = mat_ptr;

  return true;
}

bool moving_sphere::bounding_box(double _time0, double _time1,
                                 aabb &output_box) const {
  auto center0 = center.at(_time0);
  auto center1 = center.at(_time1);
  aabb box0(center0 - vec3(radius, radius, radius),
            center0 + vec3(radius, radius, radius));
  aabb box1(center1 - vec3(radius, radius, radius),
            center1 + vec3(radius, radius, radius));
  output_box = aabb(box0, box1);
  return true;
}