#pragma once

#include "hittable.h"

class hittable_list : public hittable {
 public:
  hittable_list() {}
  hittable_list(std::shared_ptr<hittable> object) { add(object); }

  void clear() { objects.clear(); }
  void add(std::shared_ptr<hittable> object) { objects.push_back(object); }

  virtual bool hit(const ray& r, const interval& ray_t,
                   hit_record& rec) const override;
  virtual bool bounding_box(double time0, double time1,
                            aabb& output_box) const override;

 public:
  std::vector<std::shared_ptr<hittable>> objects;
};
