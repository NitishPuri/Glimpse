#pragma once

#include <atomic>

#include "glimpse/film.h"
#include "glimpse/integrators.h"
#include "glimpse/util/image.h"
#include "glimpse/scenes.h"

namespace glimpse {

// declared here for testing only
vec3 sample_square_stratified(int s_i, int s_j, double recip_sqrt_spp);

class Renderer {
 public:
  void render_scene(Scene scene, Image &image, std::atomic<int> *progress = nullptr);
  static std::atomic<bool> stop_rendering;

  Film film;
};

}  // namespace glimpse