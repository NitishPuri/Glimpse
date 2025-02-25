#pragma once

#include <atomic>
#include <future>
#include <optional>

#include "core/image.h"
#include "core/render.h"
#include "core/scenes.h"

struct RayTracer {
  Scene scene;
  Image image;
  enum Status { IDLE, RENDERING, DONE };
  std::atomic<Status> status;
  std::optional<std::future<void>> trace_future;
  std::atomic<int> progress;

  void renderSceneAsync(Logger& logger, GLResources& GLResources) {
    trace_future = std::async(std::launch::async, [&]() {
      image.clear();
      logger.log("Rendering... ", GLResources.renderWidth, "x",
                 GLResources.renderHeight, " with ",
                 scene.cam.samples_per_pixel, " samples per pixel");
      auto startTime = std::chrono::high_resolution_clock::now();

      status = RENDERING;
      Renderer::render_scene(scene, image, &progress);
      status = DONE;

      auto endTime = std::chrono::high_resolution_clock::now();
      auto duration =
          std::chrono::duration<float, std::chrono::seconds::period>(endTime -
                                                                     startTime)
              .count();

      logger.log("Image generated in ", duration, " seconds");
    });
  }
};