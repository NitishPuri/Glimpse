#pragma once

#include <algorithm>

#include "../aabb.h"
#include "hittable.h"
#include "hittable_list.h"

namespace glimpse {

class bvh_node : public hittable {
 public:
  bvh_node(hittable_list list) : bvh_node(list.objects, 0, list.objects.size()) {
    // There's a C++ subtlety here. This constructor (without span indices) creates an
    // implicit copy of the hittable list, which we will modify. The lifetime of the copied
    // list only extends until this constructor exits. That's OK, because we only need to
    // persist the resulting bounding volume hierarchy.
  }

  bvh_node(std::vector<shared_ptr<hittable>>& objects, size_t start, size_t end);

  bool hit(const ray& r, interval ray_t, hit_record& rec) const override;

  aabb bounding_box() const override { return bbox; }

 private:
  shared_ptr<hittable> left;
  shared_ptr<hittable> right;
  aabb bbox;

 protected:
  static bool box_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b, int axis_index) {
    auto a_axis_interval = a->bounding_box().axis_interval(axis_index);
    auto b_axis_interval = b->bounding_box().axis_interval(axis_index);
    return a_axis_interval.min < b_axis_interval.min;
  }

  static bool box_x_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) { return box_compare(a, b, 0); }

  static bool box_y_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) { return box_compare(a, b, 1); }

  static bool box_z_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) { return box_compare(a, b, 2); }
};

}  // namespace glimpse