#pragma once

#include "glimpse/aggregates.h"
#include "glimpse/interaction.h"
#include "glimpse/util/vecmath.h"

namespace glimpse {

class Sphere : public Hittable {
 public:
  // Stationary Sphere
  Sphere(const Point3& staticCenter, double radius_, shared_ptr<Material> mat)
      : center(staticCenter, Vec3(0, 0, 0)), radius(std::fmax(0, radius_)), mat(mat) {
    auto rvec = Vec3(radius, radius, radius);
    bbox = AABB(staticCenter - rvec, staticCenter + rvec);
  }

  // Moving Sphere
  Sphere(const Point3& center1, const Point3& center2, double radius_, shared_ptr<Material> mat)
      : center(center1, center2 - center1), radius(std::fmax(0, radius_)), mat(mat) {
    auto rvec = Vec3(radius, radius, radius);
    AABB box1(center.at(0) - rvec, center.at(0) + rvec);
    AABB box2(center.at(1) - rvec, center.at(1) + rvec);
    bbox = AABB(box1, box2);
  }

  bool hit(const Ray& r, Interval rayT, HitRecord& rec) const override {
    Point3 currentCenter = center.at(r.time());
    Vec3 oc = currentCenter - r.origin();
    auto a = r.direction().lengthSquared();
    auto h = dot(r.direction(), oc);
    auto c = oc.lengthSquared() - radius * radius;

    auto discriminant = h * h - a * c;
    if (discriminant < 0) return false;

    auto sqrtd = std::sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    auto root = (h - sqrtd) / a;
    if (!rayT.surrounds(root)) {
      root = (h + sqrtd) / a;
      if (!rayT.surrounds(root)) return false;
    }

    rec.t = root;
    rec.p = r.at(rec.t);
    Vec3 outwardNormal = (rec.p - currentCenter) / radius;
    rec.setFaceNormal(r, outwardNormal);
    getSphereUv(outwardNormal, rec.u, rec.v);
    rec.mat = mat;

    return true;
  }

  AABB boundingBox() const override { return bbox; }

  double pdfValue(const Point3& origin, const Vec3& direction) const override {
    // This method only works for stationary spheres.

    HitRecord rec;
    if (!this->hit(Ray(origin, direction), Interval(0.001, math::infinity), rec)) return 0;

    auto distSquared = (center.at(0) - origin).lengthSquared();
    auto cosThetaMax = std::sqrt(1 - radius * radius / distSquared);
    auto solidAngle = 2 * math::pi * (1 - cosThetaMax);

    return 1 / solidAngle;
  }

  Vec3 random(const Point3& origin) const override {
    Vec3 direction = center.at(0) - origin;
    auto distanceSquared = direction.lengthSquared();
    ONB uvw(direction);
    return uvw.transform(randomToSphere(radius, distanceSquared));
  }

 private:
  Ray center;
  double radius;
  shared_ptr<Material> mat;
  AABB bbox;

  static void getSphereUv(const Point3& p, double& u, double& v) {
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

  static Vec3 randomToSphere(double radius, double distanceSquared) {
    auto r1 = randomDouble();
    auto r2 = randomDouble();
    auto z = 1 + r2 * (std::sqrt(1 - radius * radius / distanceSquared) - 1);

    auto phi = 2 * math::pi * r1;
    auto x = std::cos(phi) * std::sqrt(1 - z * z);
    auto y = std::sin(phi) * std::sqrt(1 - z * z);

    return Vec3(x, y, z);
  }
};

class Quad : public Hittable {
 public:
  Quad(const Point3& Q, const Vec3& u, const Vec3& v, shared_ptr<Material> mat) : Q(Q), u(u), v(v), mat(mat) {
    auto n = cross(u, v);
    normal = unitVector(n);
    D = dot(normal, Q);
    w = n / dot(n, n);

    area = n.length();

    setBoundingBox();
  }

  virtual void setBoundingBox() {
    // Compute the bounding box of all four vertices.
    auto bboxDiagonal1 = AABB(Q, Q + u + v);
    auto bboxDiagonal2 = AABB(Q + u, Q + v);
    bbox = AABB(bboxDiagonal1, bboxDiagonal2);
  }

  AABB boundingBox() const override { return bbox; }

  bool hit(const Ray& r, Interval rayT, HitRecord& rec) const override {
    auto denom = dot(normal, r.direction());

    // No hit if the ray is parallel to the plane.
    if (std::fabs(denom) < 1e-8) return false;

    // Return false if the hit point parameter t is outside the ray interval.
    auto t = (D - dot(normal, r.origin())) / denom;
    if (!rayT.contains(t)) return false;

    // Determine if the hit point lies within the planar shape using its plane coordinates.
    auto intersection = r.at(t);
    Vec3 planarHitptVector = intersection - Q;
    auto alpha = dot(w, cross(planarHitptVector, v));
    auto beta = dot(w, cross(u, planarHitptVector));

    if (!isInterior(alpha, beta, rec)) return false;

    // Ray hits the 2D shape; set the rest of the hit record and return true.
    rec.t = t;
    rec.p = intersection;
    rec.mat = mat;
    rec.setFaceNormal(r, normal);

    return true;
  }

  virtual bool isInterior(double a, double b, HitRecord& rec) const {
    Interval unitInterval = Interval(0, 1);
    // Given the hit point in plane coordinates, return false if it is outside the
    // primitive, otherwise set the hit record UV coordinates and return true.

    if (!unitInterval.contains(a) || !unitInterval.contains(b)) return false;

    rec.u = a;
    rec.v = b;
    return true;
  }

  double pdfValue(const Point3& origin, const Vec3& direction) const override {
    HitRecord rec;
    if (!this->hit(Ray(origin, direction), Interval(0.001, math::infinity), rec)) return 0;

    auto distanceSquared = rec.t * rec.t * direction.lengthSquared();
    auto cosine = std::fabs(dot(direction, rec.normal) / direction.length());

    return distanceSquared / (cosine * area);
  }

  Vec3 random(const Point3& origin) const override {
    auto p = Q + (randomDouble() * u) + (randomDouble() * v);
    return p - origin;
  }

 private:
  Point3 Q;
  Vec3 u, v;
  Vec3 w;
  shared_ptr<Material> mat;
  AABB bbox;
  Vec3 normal;
  double D;
  double area;
};

inline shared_ptr<HittableList> box(const Point3& a, const Point3& b, shared_ptr<Material> mat) {
  // Returns the 3D box (six sides) that contains the two opposite vertices a & b.

  auto sides = make_shared<HittableList>();

  // Construct the two opposite vertices with the minimum and maximum coordinates.
  auto min = Point3(std::fmin(a.x(), b.x()), std::fmin(a.y(), b.y()), std::fmin(a.z(), b.z()));
  auto max = Point3(std::fmax(a.x(), b.x()), std::fmax(a.y(), b.y()), std::fmax(a.z(), b.z()));

  auto dx = Vec3(max.x() - min.x(), 0, 0);
  auto dy = Vec3(0, max.y() - min.y(), 0);
  auto dz = Vec3(0, 0, max.z() - min.z());

  sides->add(make_shared<Quad>(Point3(min.x(), min.y(), max.z()), dx, dy, mat));   // front
  sides->add(make_shared<Quad>(Point3(max.x(), min.y(), max.z()), -dz, dy, mat));  // right
  sides->add(make_shared<Quad>(Point3(max.x(), min.y(), min.z()), -dx, dy, mat));  // back
  sides->add(make_shared<Quad>(Point3(min.x(), min.y(), min.z()), dz, dy, mat));   // left
  sides->add(make_shared<Quad>(Point3(min.x(), max.y(), max.z()), dx, -dz, mat));  // top
  sides->add(make_shared<Quad>(Point3(min.x(), min.y(), min.z()), dx, dz, mat));   // bottom

  return sides;
}

}  // namespace glimpse
