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
      logger.log("Rendering... ", GLResources.renderWidth, "x", GLResources.renderHeight, " with ",
                 scene.cam.samples_per_pixel, " samples per pixel");
      auto startTime = std::chrono::high_resolution_clock::now();

      status = RENDERING;
      Renderer::render_scene(scene, image, &progress);
      status = DONE;

      auto endTime = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration<float, std::chrono::seconds::period>(endTime - startTime).count();

      logger.log("Image generated in ", duration, " seconds");
    });
  }

  void setupScene(Logger& logger, GLResources& GLResources, int current_scene, float lookFrom[3], float lookAt[3]) {
    // Ensure current_scene is within valid range
    if (current_scene < 0 || current_scene >= Scene::SceneNames.size()) {
      logger.log("Invalid scene index: ", current_scene);
      return;
    }

    logger.log("Setting up scene ... ", current_scene, " ", Scene::SceneNames[current_scene]);
    scene = Scene::SceneMap[Scene::SceneNames[current_scene]]();

    GLResources.renderWidth = scene.cam.image_width;
    GLResources.renderHeight = static_cast<int>(scene.cam.image_width / scene.cam.aspect_ratio);
    image.initialize(GLResources.renderWidth, GLResources.renderHeight);

    auto vec3tofloat = [](const vec3& v, float f[3]) {
      f[0] = static_cast<float>(v[0]);
      f[1] = static_cast<float>(v[1]);
      f[2] = static_cast<float>(v[2]);
    };
    vec3tofloat(scene.cam.lookfrom, lookFrom);
    vec3tofloat(scene.cam.lookat, lookAt);

    GLResources.setupFramebuffer(logger);
  }
};