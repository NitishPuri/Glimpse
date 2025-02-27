
// #include "hittable.h"

// bool translate::hit(const ray& r, const interval& ray_t, hit_record& rec) const {
//   ray moved_r(r.origin() - offset, r.direction(), r.time());
//   if (!ptr->hit(moved_r, ray_t, rec)) return false;

//   rec.p += offset;

//   return true;
// }

// rotate_y::rotate_y(shared_ptr<hittable> p, double angle) : object(p) {
//   auto radians = degrees_to_radians(angle);
//   sin_theta = sin(radians);
//   cos_theta = cos(radians);
//   bbox = object->bounding_box();

//   point3 min(infinity, infinity, infinity);
//   point3 max(-infinity, -infinity, -infinity);

//   for (int i = 0; i < 2; i++) {
//     for (int j = 0; j < 2; j++) {
//       for (int k = 0; k < 2; k++) {
//         auto x = i * bbox.x.max + (1 - i) * bbox.x.min;
//         auto y = j * bbox.y.max + (1 - j) * bbox.y.min;
//         auto z = k * bbox.z.max + (1 - k) * bbox.z.min;

//         auto newx = cos_theta * x + sin_theta * z;
//         auto newz = -sin_theta * x + cos_theta * z;

//         vec3 tester(newx, y, newz);

//         for (int c = 0; c < 3; c++) {
//           min[c] = fmin(min[c], tester[c]);
//           max[c] = fmax(max[c], tester[c]);
//         }
//       }
//     }
//   }

//   bbox = aabb(min, max);
// }

// bool rotate_y::hit(const ray& r, const interval& ray_t, hit_record& rec) const {
//   // Transform the ray from world space to object space.
//   auto origin = point3((cos_theta * r.origin().x()) - (sin_theta * r.origin().z()), r.origin().y(),
//                        (sin_theta * r.origin().x()) + (cos_theta * r.origin().z()));

//   auto direction = vec3((cos_theta * r.direction().x()) - (sin_theta * r.direction().z()), r.direction().y(),
//                         (sin_theta * r.direction().x()) + (cos_theta * r.direction().z()));

//   ray rotated_r(origin, direction, r.time());

//   // Determine whether an intersection exists in object space (and if so,
//   // where).

//   if (!object->hit(rotated_r, ray_t, rec)) return false;

//   // Transform the intersection from object space back to world space.

//   auto p = rec.p;
//   auto normal = rec.normal;

//   p[0] = cos_theta * rec.p[0] + sin_theta * rec.p[2];
//   p[2] = -sin_theta * rec.p[0] + cos_theta * rec.p[2];

//   normal[0] = cos_theta * rec.normal[0] + sin_theta * rec.normal[2];
//   normal[2] = -sin_theta * rec.normal[0] + cos_theta * rec.normal[2];

//   rec.p = p;
//   rec.set_face_normal(rotated_r, normal);

//   return true;
// }