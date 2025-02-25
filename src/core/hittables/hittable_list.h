#pragma once

#include "hittable.h"

class hittable_list : public hittable {
 public:
  hittable_list() {}
  hittable_list(std::shared_ptr<hittable> object) {
    add(object);
    bbox = aabb(bbox, object->bounding_box());
  }

  void clear() { objects.clear(); }
  void add(std::shared_ptr<hittable> object) {
    objects.push_back(object);
    bbox = aabb(bbox, object->bounding_box());
  }

  virtual bool hit(const ray& r, const interval& ray_t,
                   hit_record& rec) const override;
  aabb bounding_box() const override { return bbox; }

 public:
  std::vector<std::shared_ptr<hittable>> objects;
  aabb bbox;
};
