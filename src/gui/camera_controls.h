#pragma once

// Camera manipulators for the viewer: right-drag in the render panel moves the camera. They live here, not in
// Camera, because only the viewer uses them; the renderer's camera is just a model.

#include <algorithm>
#include <cmath>

#include "glimpse/camera.h"

namespace glimpse::gui {

// Fly: translate eye and target together along the view direction (dy) and sideways (dx).
inline void fly(Camera& cam, float dx, float dy) {
  Vec3 forward = unitVector(cam.lookat - cam.lookfrom);
  Vec3 right = unitVector(cross(forward, cam.vup));

  cam.lookfrom += forward * dy + right * dx;
  cam.lookat += forward * dy + right * dx;

  cam.initialize();
}

// Orbit: move the eye on a sphere around the target (dx = azimuth, dy = polar angle, clamped away from the poles).
inline void orbit(Camera& cam, float dx, float dy) {
  Vec3 direction = cam.lookfrom - cam.lookat;
  auto radius = direction.length();

  auto theta = atan2(direction.z(), direction.x());
  auto phi = acos(direction.y() / radius);

  theta += dx;
  phi = std::clamp(phi + dy, 0.1, 3.1);

  direction[0] = radius * sin(phi) * cos(theta);
  direction[1] = radius * cos(phi);
  direction[2] = radius * sin(phi) * sin(theta);

  cam.lookfrom = cam.lookat + direction;
  cam.initialize();
}

}  // namespace glimpse::gui
