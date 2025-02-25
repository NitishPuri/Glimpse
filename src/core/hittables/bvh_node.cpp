#include "bvh_node.h"

#include <algorithm>

inline bool box_compare(const shared_ptr<hittable> a,
                        const shared_ptr<hittable> b, int axis_index) {
  auto a_axis_interval = a->bounding_box().axis_interval(axis_index);
  auto b_axis_interval = b->bounding_box().axis_interval(axis_index);
  return a_axis_interval.min < b_axis_interval.min;
}

bool box_x_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
  return box_compare(a, b, 0);
}

bool box_y_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
  return box_compare(a, b, 1);
}

bool box_z_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
  return box_compare(a, b, 2);
}

bvh_node::bvh_node(std::vector<shared_ptr<hittable>> &objects, size_t start,
                   size_t end, double time0, double time1) {
  // Build the bounding box of the span of source objects.
  bbox = aabb::empty;
  for (size_t object_index = start; object_index < end; object_index++)
    bbox = aabb(bbox, objects[object_index]->bounding_box());

  int axis = bbox.longest_axis();

  auto comparator = (axis == 0)   ? box_x_compare
                    : (axis == 1) ? box_y_compare
                                  : box_z_compare;

  size_t object_span = end - start;

  if (object_span == 1) {
    left = right = objects[start];
  } else if (object_span == 2) {
    if (comparator(objects[start], objects[start + 1])) {
      left = objects[start];
      right = objects[start + 1];
    } else {
      left = objects[start + 1];
      right = objects[start];
    }
  } else {
    std::sort(objects.begin() + start, objects.begin() + end, comparator);

    auto mid = start + object_span / 2;
    left = make_shared<bvh_node>(objects, start, mid, time0, time1);
    right = make_shared<bvh_node>(objects, mid, end, time0, time1);
  }

  bbox = aabb(left->bounding_box(), right->bounding_box());
}

bool bvh_node::bounding_box(double time0, double time1,
                            aabb &output_box) const {
  output_box = bbox;
  return true;
}

bool bvh_node::hit(const ray &r, const interval &ray_t, hit_record &rec) const {
  if (!bbox.hit(r, ray_t)) return false;

  bool hit_left = left->hit(r, ray_t, rec);
  bool hit_right =
      right->hit(r, interval{ray_t.min, hit_left ? rec.t : ray_t.max}, rec);

  return hit_left || hit_right;
}