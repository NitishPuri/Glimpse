#pragma once

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
