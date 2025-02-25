#include "bvh_node.h"

#include <algorithm>

inline bool box_compare(const shared_ptr<hittable> a,
                        const shared_ptr<hittable> b, int axis_index) {
  auto a_axis_interval = a->bounding_box().axis_interval(axis_index);
  auto b_axis_interval = b->bounding_box().axis_interval(axis_index);
  return a_axis_interval.min < b_axis_interval.min;
  // aabb box_a;
  // aabb box_b;

  // if (!a->bounding_box(0, 0, box_a) || !b->bounding_box(0, 0, box_b))
  //   std::cerr << "No bounding box in bvh_node constructor.\n";

  // return box_a.min().e[axis] < box_b.min().e[axis];
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
  // auto objects =
  //     src_objects;  // Create a modifiable array of the source scene objects

  // Build the bounding box of the span of source objects.
  bbox = aabb::empty;
  for (size_t object_index = start; object_index < end; object_index++)
    bbox = aabb(bbox, objects[object_index]->bounding_box());

  int axis = bbox.longest_axis();

  // int axis = random_int(0, 2);
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

  // aabb box_left, box_right;

  // if (!left->bounding_box(time0, time1, box_left) ||
  //     !right->bounding_box(time0, time1, box_right))
  //   std::cerr << "No bounding box in bvh_node constructor.\n";

  // box = aabb(box_left, box_right);
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