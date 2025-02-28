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
    image_height = (image_height < 1) ? 1 : image_height;

    sqrt_spp = int(std::sqrt(samples_per_pixel));
    pixel_samples_scale = 1.0 / (sqrt_spp * sqrt_spp);
    recip_sqrt_spp = 1.0 / sqrt_spp;

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

    lower_left_corner = origin - horizontal / 2 - vertical / 2 - focus_distance * w;

    // Calculate the camera defocus disk basis vectors.
    // defocus_angle = aperture;
    auto defocus_radius = focus_distance * std::tan(degrees_to_radians(defocus_angle / 2));
    defocus_disk_u = u * defocus_radius;
    defocus_disk_v = v * defocus_radius;
  }

  // normalised coordinates!
  ray get_ray(double s, double t) const {
    // without defocus blur, all rays pass through the origin

    auto ray_origin = (defocus_angle <= 0) ? origin : defocus_disk_sample();

    auto ray_direction = lower_left_corner + s * horizontal + t * vertical - ray_origin;

    // Random time for motion blur?
    return ray(ray_origin, ray_direction, random_double());
  }

  point3 defocus_disk_sample() const {
    // Returns a random point in the camera defocus disk.
    auto p = random_in_unit_disk();
    return origin + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
  }

  // TODO: Move manipulator out of base camera!
  void fly(float dx, float dy) {
    vec3 forward = unit_vector(lookat - lookfrom);
    vec3 right = unit_vector(cross(forward, vup));
    vec3 up = cross(right, forward);

    lookfrom += forward * dy + right * dx;
    lookat += forward * dy + right * dx;

    initialize();
  }

  void orbit(float dx, float dy) {
    vec3 direction = lookfrom - lookat;
    auto radius = direction.length();

    auto theta = atan2(direction.z(), direction.x());
    auto phi = acos(direction.y() / radius);

    theta += dx;
    phi = std::clamp(phi + dy, 0.1, 3.1);

    direction[0] = radius * sin(phi) * cos(theta);
    direction[1] = radius * cos(phi);
    direction[2] = radius * sin(phi) * sin(theta);

    lookfrom = lookat + direction;
    initialize();
  }

 public:
  double aspect_ratio = 16.0 / 9.0;  // Ratio of image width over height
  int image_width = 800;             // rendered image width in pixels

  // SPP
  bool uncapped_spp = false;     // If true, the number of samples per pixel is
                                 // not capped at sqrt_spp
  double pixel_samples_scale{};  // Color scale factor for a sum of pixel samples
  int samples_per_pixel = 100;   // count of random samples for each pixel
  int sqrt_spp{};                // Square root of number of samples per pixel
  double recip_sqrt_spp{};       // 1/sqrt_spp

  int max_depth = 50;  // maximum number of ray bounces into scene

  float vfov = 60.0f;                 // vertical field of view
  point3 lookfrom = point3(0, 0, 0);  // point camera is looking from
  point3 lookat = point3(0, 0, -1);   // point camera is looking at
  point3 vup = vec3(0, 1, 0);         // camera realtive "up" direction

  float defocus_angle = 0.1f;    // variation angle of rays through each pixel
  float focus_distance = 10.0f;  // distance from camera lookfrom point to plane of perfect focus

  int image_height = 0;

 private:
  point3 origin;

  point3 lower_left_corner;
  vec3 horizontal;
  vec3 vertical;

  vec3 u, v, w;
  vec3 defocus_disk_u, defocus_disk_v;
};