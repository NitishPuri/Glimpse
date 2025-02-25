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

#define DEFOCUS_IMPL 1
// Options :: 0 = isotropic blur, 1 = anisotropic defocus blur

class camera {
 public:
  void initialize() {
    image_height = static_cast<int>(image_width / aspect_ratio);
    image_height = (image_height < 1) ? 1 : image_height;

    pixel_samples_scale = 1.0 / samples_per_pixel;
    origin = lookfrom;

    // Determine viewport dimensions.
    auto theta = degrees_to_radians(vfov);
    auto h = tan(theta / 2);
    auto viewport_height = 2.0 * h * focus_distance;
    auto viewport_width = aspect_ratio * viewport_height;

    // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
    w = unit_vector(lookfrom - lookat);
    u = unit_vector(cross(vup, w));
    v = cross(w, u);

    // Calculate the vectors across the horizontal and down the vertical
    // viewport edges.
    horizontal = viewport_width * u;
    vertical = viewport_height * v;  // TODO: should this be inverted ?

    lower_left_corner =
        origin - horizontal / 2 - vertical / 2 - focus_distance * w;

#if DEFOCUS_IMPL == 0
    lens_radius = aperture / 2;
#else
    // Calculate the camera defocus disk basis vectors.
    defocus_angle = aperture;
    auto defocus_radius =
        focus_distance * std::tan(degrees_to_radians(defocus_angle / 2));
    defocus_disk_u = u * defocus_radius;
    defocus_disk_v = v * defocus_radius;
#endif
  }

  // normalised coordinates!
  ray get_ray(double s, double t) const {
    // without defocus blur, all rays pass through the origin

#if DEFOCUS_IMPL == 0
    vec3 rd = lens_radius * random_in_unit_disk();
    vec3 offset = u * rd.x() + v * rd.y();

    auto ray_origin = origin + offset;
#else
    auto ray_origin = (defocus_angle <= 0) ? origin : defocus_disk_sample();
#endif

    auto ray_direction =
        lower_left_corner + s * horizontal + t * vertical - ray_origin;

    // Random time for motion blur?
    return ray(ray_origin, ray_direction, random_double(time0, time1));
  }

#if DEFOCUS_IMPL == 1
  point3 defocus_disk_sample() const {
    // Returns a random point in the camera defocus disk.
    auto p = random_in_unit_disk();
    return origin + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
  }
#endif

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

  double time0 = 0.0, time1 = 1.0;  // default shutter open/close times

  int image_height = 0;
  double pixel_samples_scale;  // Color scale factor for a sum of pixel samples

 private:
  point3 origin;

  point3 lower_left_corner;
  vec3 horizontal;
  vec3 vertical;

  vec3 u, v, w;

#if DEFOCUS_IMPL == 0
  double lens_radius;
#elif DEFOCUS_IMPL == 1
  vec3 defocus_disk_u, defocus_disk_v;
#endif
};