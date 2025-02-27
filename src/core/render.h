#pragma once

#include <atomic>

#include "film.h"
#include "image.h"
#include "scenes.h"

class Renderer {
 public:
  void render_scene(const Scene &scene, Image &image, std::atomic<int> *progress = nullptr);

 private:
  film film;
};