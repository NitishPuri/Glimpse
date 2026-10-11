#pragma once

#include <algorithm>
#include <vector>

#include "glimpse/interaction.h"
#include "glimpse/util/aabb.h"

namespace glimpse {

class hittable_list : public hittable {
 public:
  std::vector<shared_ptr<hittable>> objects;

  hittable_list() {}
  hittable_list(shared_ptr<hittable> object) { add(object); }

  void clear() { objects.clear(); }

  void add(shared_ptr<hittable> object) {
    objects.push_back(object);
    bbox = aabb(bbox, object->bounding_box());
  }

  bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
    hit_record temp_rec;
    bool hit_anything = false;
    auto closest_so_far = ray_t.max;

    for (const auto& object : objects) {
      if (object->hit(r, interval(ray_t.min, closest_so_far), temp_rec)) {
        hit_anything = true;
        closest_so_far = temp_rec.t;
        rec = temp_rec;
      }
    }

    return hit_anything;
  }

  aabb bounding_box() const override { return bbox; }

  double pdf_value(const point3& origin, const vec3& direction) const override {
    // ASSERT(objects.size() > 0);
    if (objects.empty()) return 0.0;
    auto weight = 1.0 / objects.size();
    auto sum = 0.0;

    for (const auto& object : objects) sum += weight * object->pdf_value(origin, direction);

    return sum;
  }

  vec3 random(const point3& origin) const override {
    // ASSERT(objects.size() > 0);
    if (objects.empty()) return vec3(1, 0, 0);
    // if (objects.empty()) return vec3(0, 0, 0);
    auto int_size = int(objects.size());
    return objects[random_int(0, int_size - 1)]->random(origin);
  }

 private:
  aabb bbox;
};

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
