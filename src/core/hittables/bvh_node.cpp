#include "bvh_node.h"

#include <algorithm>

using namespace glimpse;

bvh_node::bvh_node(std::vector<shared_ptr<hittable>>& objects, size_t start, size_t end) {
  bbox = aabb::empty;
  if (start == end) return;
  // Build the bounding box of the span of source objects.
  for (size_t object_index = start; object_index < end; object_index++)
    bbox = aabb(bbox, objects[object_index]->bounding_box());

  int axis = bbox.longest_axis();

  auto comparator = (axis == 0) ? box_x_compare : (axis == 1) ? box_y_compare : box_z_compare;

  size_t object_span = end - start;

  if (object_span == 1) {
    left = right = objects[start];
  } else if (object_span == 2) {
    left = objects[start];
    right = objects[start + 1];
  } else {
    std::sort(std::begin(objects) + start, std::begin(objects) + end, comparator);

    auto mid = start + object_span / 2;
    left = make_shared<bvh_node>(objects, start, mid);
    right = make_shared<bvh_node>(objects, mid, end);
  }
}

bool bvh_node::hit(const ray& r, interval ray_t, hit_record& rec) const {
  if (!bbox.hit(r, ray_t)) return false;

  bool hit_left = left->hit(r, ray_t, rec);
  bool hit_right = right->hit(r, interval(ray_t.min, hit_left ? rec.t : ray_t.max), rec);

  return hit_left || hit_right;
}