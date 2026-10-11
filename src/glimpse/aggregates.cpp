// BVHNode construction: sort the primitives along the box's longest axis and split them in half.

#include "glimpse/aggregates.h"

#include <algorithm>

namespace glimpse {

BVHNode::BVHNode(std::vector<shared_ptr<Hittable>>& objects, size_t start, size_t end) {
  bbox = AABB::empty;
  if (start == end) return;
  // Build the bounding box of the span of source objects.
  for (size_t objectIndex = start; objectIndex < end; objectIndex++)
    bbox = AABB(bbox, objects[objectIndex]->boundingBox());

  int axis = bbox.longestAxis();

  auto comparator = (axis == 0) ? boxXCompare : (axis == 1) ? boxYCompare : boxZCompare;

  size_t objectSpan = end - start;

  if (objectSpan == 1) {
    left = right = objects[start];
  } else if (objectSpan == 2) {
    left = objects[start];
    right = objects[start + 1];
  } else {
    std::sort(std::begin(objects) + start, std::begin(objects) + end, comparator);

    auto mid = start + objectSpan / 2;
    left = make_shared<BVHNode>(objects, start, mid);
    right = make_shared<BVHNode>(objects, mid, end);
  }
}

bool BVHNode::hit(const Ray& r, Interval rayT, HitRecord& rec) const {
  if (!bbox.hit(r, rayT)) return false;

  bool hitLeft = left->hit(r, rayT, rec);
  bool hitRight = right->hit(r, Interval(rayT.min, hitLeft ? rec.t : rayT.max), rec);

  return hitLeft || hitRight;
}

}  // namespace glimpse
