#pragma once

#include <atomic>

#include "film.h"
#include "image.h"
#include "scenes.h"

class Renderer {
 public:
  void render_scene(const Scene &scene, Image &image, std::atomic<int> *progress = nullptr);
  static std::atomic<bool> stop_rendering;

  Film film;
};
