#pragma once

#include "glimpse/interaction.h"
#include "glimpse/materials.h"
#include "glimpse/textures.h"

namespace glimpse {

class ConstantMedium : public Hittable {
 public:
  ConstantMedium(shared_ptr<Hittable> boundary, double density, shared_ptr<Texture> tex)
      : boundary(boundary), negInvDensity(-1 / density), phaseFunction(make_shared<Isotropic>(tex)) {}

  ConstantMedium(shared_ptr<Hittable> boundary, double density, const Color& albedo)
      : boundary(boundary), negInvDensity(-1 / density), phaseFunction(make_shared<Isotropic>(albedo)) {}

  bool hit(const Ray& r, Interval rayT, HitRecord& rec) const override {
    HitRecord rec1, rec2;

    if (!boundary->hit(r, Interval::universe, rec1)) return false;

    if (!boundary->hit(r, Interval(rec1.t + 0.0001, math::infinity), rec2)) return false;

    if (rec1.t < rayT.min) rec1.t = rayT.min;
    if (rec2.t > rayT.max) rec2.t = rayT.max;

    if (rec1.t >= rec2.t) return false;

    if (rec1.t < 0) rec1.t = 0;

    auto rayLength = r.direction().length();
    auto distanceInsideBoundary = (rec2.t - rec1.t) * rayLength;
    auto hitDistance = negInvDensity * std::log(randomDouble());

    if (hitDistance > distanceInsideBoundary) return false;

    rec.t = rec1.t + hitDistance / rayLength;
    rec.p = r.at(rec.t);

    rec.normal = Vec3(1, 0, 0);  // arbitrary
    rec.frontFace = true;       // also arbitrary
    rec.mat = phaseFunction;

    return true;
  }

  AABB boundingBox() const override { return boundary->boundingBox(); }

 private:
  shared_ptr<Hittable> boundary;
  double negInvDensity;
  shared_ptr<Material> phaseFunction;
};

}  // namespace glimpse
