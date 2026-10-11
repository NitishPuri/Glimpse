#pragma once

#include "glimpse/aggregates.h"
#include "glimpse/interaction.h"
#include "glimpse/util/vecmath.h"

namespace glimpse {

class sphere : public hittable {
 public:
  // Stationary Sphere
  sphere(const point3& static_center, double radius_, shared_ptr<material> mat)
      : center(static_center, vec3(0, 0, 0)), radius(std::fmax(0, radius_)), mat(mat) {
    auto rvec = vec3(radius, radius, radius);
    bbox = aabb(static_center - rvec, static_center + rvec);
  }

  // Moving Sphere
  sphere(const point3& center1, const point3& center2, double radius_, shared_ptr<material> mat)
      : center(center1, center2 - center1), radius(std::fmax(0, radius_)), mat(mat) {
    auto rvec = vec3(radius, radius, radius);
    aabb box1(center.at(0) - rvec, center.at(0) + rvec);
    aabb box2(center.at(1) - rvec, center.at(1) + rvec);
    bbox = aabb(box1, box2);
  }

  bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
    point3 current_center = center.at(r.time());
    vec3 oc = current_center - r.origin();
    auto a = r.direction().length_squared();
    auto h = dot(r.direction(), oc);
    auto c = oc.length_squared() - radius * radius;

    auto discriminant = h * h - a * c;
    if (discriminant < 0) return false;

    auto sqrtd = std::sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    auto root = (h - sqrtd) / a;
    if (!ray_t.surrounds(root)) {
      root = (h + sqrtd) / a;
      if (!ray_t.surrounds(root)) return false;
    }

    rec.t = root;
    rec.p = r.at(rec.t);
    vec3 outward_normal = (rec.p - current_center) / radius;
    rec.set_face_normal(r, outward_normal);
    get_sphere_uv(outward_normal, rec.u, rec.v);
    rec.mat = mat;

    return true;
  }

  aabb bounding_box() const override { return bbox; }

  double pdf_value(const point3& origin, const vec3& direction) const override {
    // This method only works for stationary spheres.

    hit_record rec;
    if (!this->hit(ray(origin, direction), interval(0.001, math::infinity), rec)) return 0;

    auto dist_squared = (center.at(0) - origin).length_squared();
    auto cos_theta_max = std::sqrt(1 - radius * radius / dist_squared);
    auto solid_angle = 2 * math::pi * (1 - cos_theta_max);

    return 1 / solid_angle;
  }

  vec3 random(const point3& origin) const override {
    vec3 direction = center.at(0) - origin;
    auto distance_squared = direction.length_squared();
    onb uvw(direction);
    return uvw.transform(random_to_sphere(radius, distance_squared));
  }

 private:
  ray center;
  double radius;
  shared_ptr<material> mat;
  aabb bbox;

  static void get_sphere_uv(const point3& p, double& u, double& v) {
    // p: a given point on the sphere of radius one, centered at the origin.
    // u: returned value [0,1] of angle around the Y axis from X=-1.
    // v: returned value [0,1] of angle from Y=-1 to Y=+1.
    //     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
    //     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
    //     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>

    auto theta = std::acos(-p.y());
    auto phi = std::atan2(-p.z(), p.x()) + math::pi;

    u = phi / (2 * math::pi);
    v = theta / math::pi;
  }

  static vec3 random_to_sphere(double radius, double distance_squared) {
    auto r1 = random_double();
    auto r2 = random_double();
    auto z = 1 + r2 * (std::sqrt(1 - radius * radius / distance_squared) - 1);

    auto phi = 2 * math::pi * r1;
    auto x = std::cos(phi) * std::sqrt(1 - z * z);
    auto y = std::sin(phi) * std::sqrt(1 - z * z);

    return vec3(x, y, z);
  }
};

class moving_sphere : public hittable {
 public:
  moving_sphere() {}
  moving_sphere(point3 cen0, point3 cen1, double r, shared_ptr<material> m)
      : center{cen0, cen1 - cen0}, radius{r}, mat_ptr{m} {
    auto rvec = vec3(radius, radius, radius);
    aabb box1(center.at(0) - rvec, center.at(0) + rvec);
    aabb box2(center.at(1) - rvec, center.at(1) + rvec);
    bbox = aabb(box1, box2);
  }

  bool hit(const ray &r, interval ray_t, hit_record &rec) const override;

  aabb bounding_box() const override { return bbox; }

 public:
  ray center;
  double time0{}, time1{};
  double radius{};
  aabb bbox;
  shared_ptr<material> mat_ptr;
};

class quad : public hittable {
 public:
  quad(const point3& Q, const vec3& u, const vec3& v, shared_ptr<material> mat) : Q(Q), u(u), v(v), mat(mat) {
    auto n = cross(u, v);
    normal = unit_vector(n);
    D = dot(normal, Q);
    w = n / dot(n, n);

    area = n.length();

    set_bounding_box();
  }

  virtual void set_bounding_box() {
    // Compute the bounding box of all four vertices.
    auto bbox_diagonal1 = aabb(Q, Q + u + v);
    auto bbox_diagonal2 = aabb(Q + u, Q + v);
    bbox = aabb(bbox_diagonal1, bbox_diagonal2);
  }

  aabb bounding_box() const override { return bbox; }

  bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
    auto denom = dot(normal, r.direction());

    // No hit if the ray is parallel to the plane.
    if (std::fabs(denom) < 1e-8) return false;

    // Return false if the hit point parameter t is outside the ray interval.
    auto t = (D - dot(normal, r.origin())) / denom;
    if (!ray_t.contains(t)) return false;

    // Determine if the hit point lies within the planar shape using its plane coordinates.
    auto intersection = r.at(t);
    vec3 planar_hitpt_vector = intersection - Q;
    auto alpha = dot(w, cross(planar_hitpt_vector, v));
    auto beta = dot(w, cross(u, planar_hitpt_vector));

    if (!is_interior(alpha, beta, rec)) return false;

    // Ray hits the 2D shape; set the rest of the hit record and return true.
    rec.t = t;
    rec.p = intersection;
    rec.mat = mat;
    rec.set_face_normal(r, normal);

    return true;
  }

  virtual bool is_interior(double a, double b, hit_record& rec) const {
    interval unit_interval = interval(0, 1);
    // Given the hit point in plane coordinates, return false if it is outside the
    // primitive, otherwise set the hit record UV coordinates and return true.

    if (!unit_interval.contains(a) || !unit_interval.contains(b)) return false;

    rec.u = a;
    rec.v = b;
    return true;
  }

  double pdf_value(const point3& origin, const vec3& direction) const override {
    hit_record rec;
    if (!this->hit(ray(origin, direction), interval(0.001, math::infinity), rec)) return 0;

    auto distance_squared = rec.t * rec.t * direction.length_squared();
    auto cosine = std::fabs(dot(direction, rec.normal) / direction.length());

    return distance_squared / (cosine * area);
  }

  vec3 random(const point3& origin) const override {
    auto p = Q + (random_double() * u) + (random_double() * v);
    return p - origin;
  }

 private:
  point3 Q;
  vec3 u, v;
  vec3 w;
  shared_ptr<material> mat;
  aabb bbox;
  vec3 normal;
  double D;
  double area;
};

inline shared_ptr<hittable_list> box(const point3& a, const point3& b, shared_ptr<material> mat) {
  // Returns the 3D box (six sides) that contains the two opposite vertices a & b.

  auto sides = make_shared<hittable_list>();

  // Construct the two opposite vertices with the minimum and maximum coordinates.
  auto min = point3(std::fmin(a.x(), b.x()), std::fmin(a.y(), b.y()), std::fmin(a.z(), b.z()));
  auto max = point3(std::fmax(a.x(), b.x()), std::fmax(a.y(), b.y()), std::fmax(a.z(), b.z()));

  auto dx = vec3(max.x() - min.x(), 0, 0);
  auto dy = vec3(0, max.y() - min.y(), 0);
  auto dz = vec3(0, 0, max.z() - min.z());

  sides->add(make_shared<quad>(point3(min.x(), min.y(), max.z()), dx, dy, mat));   // front
  sides->add(make_shared<quad>(point3(max.x(), min.y(), max.z()), -dz, dy, mat));  // right
  sides->add(make_shared<quad>(point3(max.x(), min.y(), min.z()), -dx, dy, mat));  // back
  sides->add(make_shared<quad>(point3(min.x(), min.y(), min.z()), dz, dy, mat));   // left
  sides->add(make_shared<quad>(point3(min.x(), max.y(), max.z()), dx, -dz, mat));  // top
  sides->add(make_shared<quad>(point3(min.x(), min.y(), min.z()), dx, dz, mat));   // bottom

  return sides;
}

}  // namespace glimpse
