#pragma once

#include "glimpse/ray.h"
#include "glimpse/util/sampling.h"

namespace glimpse {

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

class Camera {
 public:
  void initialize() {
    imageHeight = static_cast<int>(imageWidth / aspectRatio);
    imageHeight = (imageHeight < 1) ? 1 : imageHeight;

    sqrtSpp = int(std::sqrt(samplesPerPixel));
    pixelSamplesScale = 1.0 / (sqrtSpp * sqrtSpp);
    recipSqrtSpp = 1.0 / sqrtSpp;

    origin = lookfrom;

    // Determine viewport dimensions.
    auto theta = math::degreesToRadians(vfov);
    auto h = tan(theta / 2);
    auto viewportHeight = 2.0 * h * focusDistance;
    auto viewportWidth = aspectRatio * viewportHeight;

    // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
    w = unitVector(lookfrom - lookat);
    u = unitVector(cross(vup, w));
    v = cross(w, u);

    // Calculate the vectors across the horizontal and down the vertical
    // viewport edges.
    horizontal = viewportWidth * u;
    vertical = viewportHeight * v;  // +v is up: getRay(s, t) has t = 0 at the bottom edge. Image rows are top-down,
                                    // so the renderer flips rows when it writes the Image (render.cpp).

    lowerLeftCorner = origin - horizontal / 2 - vertical / 2 - focusDistance * w;

    // Calculate the camera defocus disk basis vectors.
    auto defocusRadius = focusDistance * std::tan(math::degreesToRadians(defocusAngle / 2));
    defocusDiskU = u * defocusRadius;
    defocusDiskV = v * defocusRadius;
  }

  // normalised coordinates!
  Ray getRay(double s, double t) const {
    // without defocus blur, all rays pass through the origin

    auto rayOrigin = (defocusAngle <= 0) ? origin : defocusDiskSample();

    auto rayDirection = lowerLeftCorner + s * horizontal + t * vertical - rayOrigin;

    // Random time for motion blur?
    return Ray(rayOrigin, rayDirection, randomDouble());
  }

  Point3 defocusDiskSample() const {
    // Returns a random point in the camera defocus disk.
    auto p = randomInUnitDisk();
    return origin + (p[0] * defocusDiskU) + (p[1] * defocusDiskV);
  }

 public:
  double aspectRatio = 16.0 / 9.0;  // Ratio of image width over height
  int imageWidth = 800;             // rendered image width in pixels

  // SPP
  bool uncappedSpp = false;     // If true, the number of samples per pixel is
                                 // not capped at sqrtSpp
  double pixelSamplesScale{};  // Color scale factor for a sum of pixel samples
  int samplesPerPixel = 100;   // count of random samples for each pixel
  int sqrtSpp{};                // Square root of number of samples per pixel
  double recipSqrtSpp{};       // 1/sqrtSpp

  int maxDepth = 50;  // maximum number of ray bounces into scene

  float vfov = 60.0f;                 // vertical field of view
  Point3 lookfrom = Point3(0, 0, 0);  // point camera is looking from
  Point3 lookat = Point3(0, 0, -1);   // point camera is looking at
  Point3 vup = Vec3(0, 1, 0);         // camera realtive "up" direction

  float defocusAngle = 0.1f;    // variation angle of rays through each pixel
  float focusDistance = 10.0f;  // distance from camera lookfrom point to plane of perfect focus

  int imageHeight = 0;

 private:
  Point3 origin;

  Point3 lowerLeftCorner;
  Vec3 horizontal;
  Vec3 vertical;

  Vec3 u, v, w;
  Vec3 defocusDiskU, defocusDiskV;
};

}  // namespace glimpse