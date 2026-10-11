#pragma once

#include <algorithm>
#include <vector>

#include "glimpse/interaction.h"
#include "glimpse/util/aabb.h"

namespace glimpse {

class HittableList : public Hittable {
 public:
  std::vector<shared_ptr<Hittable>> objects;

  HittableList() {}
  HittableList(shared_ptr<Hittable> object) { add(object); }

  void clear() { objects.clear(); }

  void add(shared_ptr<Hittable> object) {
    objects.push_back(object);
    bbox = AABB(bbox, object->boundingBox());
  }

  bool hit(const Ray& r, Interval rayT, HitRecord& rec) const override {
    HitRecord tempRec;
    bool hitAnything = false;
    auto closestSoFar = rayT.max;

    for (const auto& object : objects) {
      if (object->hit(r, Interval(rayT.min, closestSoFar), tempRec)) {
        hitAnything = true;
        closestSoFar = tempRec.t;
        rec = tempRec;
      }
    }

    return hitAnything;
  }

  AABB boundingBox() const override { return bbox; }

  double pdfValue(const Point3& origin, const Vec3& direction) const override {
    if (objects.empty()) return 0.0;
    auto weight = 1.0 / objects.size();
    auto sum = 0.0;

    for (const auto& object : objects) sum += weight * object->pdfValue(origin, direction);

    return sum;
  }

  Vec3 random(const Point3& origin) const override {
    if (objects.empty()) return Vec3(1, 0, 0);
    auto intSize = int(objects.size());
    return objects[randomInt(0, intSize - 1)]->random(origin);
  }

 private:
  AABB bbox;
};

class BVHNode : public Hittable {
 public:
  BVHNode(HittableList list) : BVHNode(list.objects, 0, list.objects.size()) {
    // There's a C++ subtlety here. This constructor (without span indices) creates an
    // implicit copy of the hittable list, which we will modify. The lifetime of the copied
    // list only extends until this constructor exits. That's OK, because we only need to
    // persist the resulting bounding volume hierarchy.
  }

  BVHNode(std::vector<shared_ptr<Hittable>>& objects, size_t start, size_t end);

  bool hit(const Ray& r, Interval rayT, HitRecord& rec) const override;

  AABB boundingBox() const override { return bbox; }

 private:
  shared_ptr<Hittable> left;
  shared_ptr<Hittable> right;
  AABB bbox;

 protected:
  static bool boxCompare(const shared_ptr<Hittable> a, const shared_ptr<Hittable> b, int axisIndex) {
    auto aAxisInterval = a->boundingBox().axisInterval(axisIndex);
    auto bAxisInterval = b->boundingBox().axisInterval(axisIndex);
    return aAxisInterval.min < bAxisInterval.min;
  }

  static bool boxXCompare(const shared_ptr<Hittable> a, const shared_ptr<Hittable> b) { return boxCompare(a, b, 0); }

  static bool boxYCompare(const shared_ptr<Hittable> a, const shared_ptr<Hittable> b) { return boxCompare(a, b, 1); }

  static bool boxZCompare(const shared_ptr<Hittable> a, const shared_ptr<Hittable> b) { return boxCompare(a, b, 2); }
};

}  // namespace glimpse
