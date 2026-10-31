#pragma once

// Renderer: drives a render. It builds the BVH, splits the image across threads, and accumulates samples into the
// Film and the displayed Image.

#include <atomic>
#include <cstdint>

#include "glimpse/film.h"
#include "glimpse/integrators.h"
#include "glimpse/util/image.h"
#include "glimpse/scene.h"

namespace glimpse {

// declared here for testing only
Vec3 sampleSquareStratified(int sI, int sJ, double recipSqrtSpp);

class Renderer {
 public:
  void renderScene(Scene scene, Image &image, std::atomic<int64_t> *progress = nullptr);
  static std::atomic<bool> stopRequested;

  Film film;
};

}  // namespace glimpse