#pragma once

#include "ray.h"

// 1. focus plane is orthogonal to the camera
// 2. focus distance is the distance between camera center and focus plane
// 3. viewport lies on the focus plane centeered on the camera view direction
//    vector
// 4. The grid of pixel locations lies inside the viewport (located in the 3D
//    world)
// 5. random image sample locations are choosen from the region around
//    the current pixel location
// 6. The camera fires rays from random points on the lens
//    through the current image sample.

class camera {
 public:
  void initialize() {
    image_height = static_cast<int>(image_width / aspect_ratio);

    auto theta = degrees_to_radians(vfov);
    auto h = tan(theta / 2);
    auto viewport_height = 2.0 * h;
    auto viewport_width = aspect_ratio * viewport_height;

    w = unit_vector(lookfrom - lookat);
    u = unit_vector(cross(vup, w));
    v = cross(w, u);

    origin = lookfrom;
    horizontal = focus_distance * viewport_width * u;
    vertical = focus_distance * viewport_height * v;
    lower_left_corner =
        origin - horizontal / 2 - vertical / 2 - focus_distance * w;

    lens_radius = aperture / 2;
  }

  ray get_ray(double s, double t) const {
    // without defocus blur, all rays pass through the origin
    vec3 rd = lens_radius * random_in_unit_disk();
    vec3 offset = u * rd.x() + v * rd.y();

    auto ray_origin = origin + offset;
    auto ray_direction =
        lower_left_corner + s * horizontal + t * vertical - ray_origin;
    // Random time for motion blur?
    return ray(ray_origin, ray_direction, random_double(time0, time1));
  }

 public:
  double aspect_ratio = 16.0 / 9.0;  // Ratio of image width over height
  int image_width = 800;             // rendered image width in pixels
  int samples_per_pixel = 100;       // count of random samples for each pixel
  int max_depth = 50;                // maximum number of ray bounces into scene

  float vfov = 60.0f;                 // vertical field of view
  point3 lookfrom = point3(0, 0, 0);  // point camera is looking from
  point3 lookat = point3(0, 0, -1);   // point camera is looking at
  point3 vup = vec3(0, 1, 0);         // camera realtive "up" direction

  float aperture = 0.1f;    //
  float defocus_angle = 0;  // variation angle of rays through each pixel
  float focus_distance =
      10.0f;  // distance from camera lookfrom point to plane of perfect focus

  double time0, time1;  // shutter open/close times

  int image_height = 0;

 private:
  point3 origin;
  point3 lower_left_corner;
  vec3 horizontal;
  vec3 vertical;
  vec3 u, v, w;
  double lens_radius;
};