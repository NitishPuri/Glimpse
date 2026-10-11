#pragma once

#include "glimpse/camera.h"
#include "glimpse/aggregates.h"

namespace glimpse {

struct Scene {
  // world
  hittable_list world;

  hittable_list lights;

  // env
  color background = color(0.7, 0.8, 1.0);

  camera cam;
};

}  // namespace glimpse
