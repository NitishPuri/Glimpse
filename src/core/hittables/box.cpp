
#include "box.h"

#include "aarect.h"

box::box(const point3 &p0, const point3 &p1, shared_ptr<material> ptr)
    : bbox(p0, p1) {
  box_min = p0;
  box_max = p1;

  sides.add(make_shared<xy_rect>(p0.x(), p1.x(), p0.y(), p1.y(), p1.z(), ptr));
  sides.add(make_shared<xy_rect>(p0.x(), p1.x(), p0.y(), p1.y(), p0.z(), ptr));

  sides.add(make_shared<xz_rect>(p0.x(), p1.x(), p0.z(), p1.z(), p1.y(), ptr));
  sides.add(make_shared<xz_rect>(p0.x(), p1.x(), p0.z(), p1.z(), p0.y(), ptr));

  sides.add(make_shared<yz_rect>(p0.y(), p1.y(), p0.z(), p1.z(), p1.x(), ptr));
  sides.add(make_shared<yz_rect>(p0.y(), p1.y(), p0.z(), p1.z(), p0.x(), ptr));
}

bool box::bounding_box(double time0, double time1, aabb &output_box) const {
  output_box = aabb(box_min, box_max);
  return true;
}

bool box::hit(const ray &r, const interval &ray_t, hit_record &rec) const {
  return sides.hit(r, {ray_t.min, ray_t.max}, rec);
}