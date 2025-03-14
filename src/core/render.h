#pragma once

#include <atomic>

#include "film.h"
#include "image.h"
#include "scenes.h"

namespace glimpse {

// declared here for testing only
color ray_color(const ray &r, const color &background, const hittable &world,  //
                int depth, const hittable &lights, bool has_lights);
vec3 sample_square_stratified(int s_i, int s_j, double recip_sqrt_spp);

class Renderer {
 public:
  void render_scene(Scene scene, Image &image, std::atomic<int> *progress = nullptr);
  static std::atomic<bool> stop_rendering;

  Film film;
};

}  // namespace glimpse