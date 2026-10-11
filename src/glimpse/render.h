#pragma once

#include <atomic>

#include "glimpse/film.h"
#include "glimpse/integrators.h"
#include "glimpse/util/image.h"
#include "glimpse/scene.h"

namespace glimpse {

// declared here for testing only
Vec3 sampleSquareStratified(int sI, int sJ, double recipSqrtSpp);

class Renderer {
 public:
  void renderScene(Scene scene, Image &image, std::atomic<int> *progress = nullptr);
  static std::atomic<bool> stopRequested;

  Film film;
};

}  // namespace glimpse