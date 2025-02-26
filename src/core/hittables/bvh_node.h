#pragma once

#include "hittable_list.h"

class bvh_node : public hittable {
 public:
  bvh_node();

  bvh_node(std::vector<shared_ptr<hittable>> &objects, size_t start, size_t end);

  bvh_node(hittable_list list) : bvh_node(list.objects, 0, list.objects.size()) {
    // There's a C++ subtlety here. This constructor (without span indices)
    // creates an implicit copy of the hittable list, which we will modify. The
    // lifetime of the copied list only extends until this constructor exits.
    // That's OK, because we only need to persist the resulting bounding volume
    // hierarchy.
  }

  bool hit(const ray &r, const interval &ray_t, hit_record &rec) const override;

  aabb bounding_box() const override { return bbox; }

 public:
  shared_ptr<hittable> left;
  shared_ptr<hittable> right;
  aabb bbox;
};
