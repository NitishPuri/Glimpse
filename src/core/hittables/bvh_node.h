#pragma once

#include "hittable_list.h"

class bvh_node : public hittable {
 public:
  bvh_node();

  bvh_node(hittable_list list, double time0, double time1)
      : bvh_node(list.objects, 0, list.objects.size(), time0, time1) {
    // There's a C++ subtlety here. This constructor (without span indices)
    // creates an implicit copy of the hittable list, which we will modify. The
    // lifetime of the copied list only extends until this constructor exits.
    // That's OK, because we only need to persist the resulting bounding volume
    // hierarchy.
  }

  bvh_node(std::vector<shared_ptr<hittable>> &objects, size_t start, size_t end,
           double time0, double time1);

  bool hit(const ray &r, const interval &ray_t, hit_record &rec) const override;

  bool bounding_box(double time0, double time1,
                    aabb &output_box) const override;

  aabb bounding_box() const override { return bbox; }

 public:
  shared_ptr<hittable> left;
  shared_ptr<hittable> right;
  aabb bbox;
};
