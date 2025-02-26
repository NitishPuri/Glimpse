#pragma once

#include "image.h"
#include "scenes.h"

class Renderer {
 public:
  static void render_scene(const Scene &scene, Image &image, const hittable &lights,
                           std::atomic<int> *progress = nullptr);
};