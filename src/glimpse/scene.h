#pragma once

// Scene: what a render needs: world geometry, the lights to sample (duplicates of emitters in the world), a
// background color and the camera.

#include "glimpse/camera.h"
#include "glimpse/aggregates.h"

namespace glimpse {

struct Scene {
  // world
  HittableList world;

  HittableList lights;

  // env
  Color background = Color(0.7, 0.8, 1.0);

  Camera cam;
};

}  // namespace glimpse
