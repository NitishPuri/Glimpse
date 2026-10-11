#pragma once

#include <atomic>
#include <future>
#include <optional>

#include "glimpse/shapes.h"
#include "glimpse/util/image.h"
#include "glimpse/render.h"
#include "glimpse/scenes/registry.h"

namespace glimpse::gui {

struct RayTracer {
  Scene scene;
  Image image;
  enum Status { IDLE, RENDERING, DONE };
  std::atomic<Status> status{IDLE};
  std::optional<std::future<void>> traceFuture{};
  std::atomic<int> progress{0};
  // Samples per pixel of the last finished render (for the saved file name). Kept here because `renderer`, and
  // with it the film's sample counts, is released when a render completes.
  std::atomic<int> lastSpp{0};

  std::shared_ptr<Renderer> renderer;

  Logger& logger;
  RayTracer(Logger& logger) : logger(logger) {}

  void renderSceneAsync() {
    // check if we are already rendering, if yes, then lets do nothing for now.
    if (renderer.get() != nullptr) {
      return;
    }

    renderer = std::make_shared<Renderer>();
    // A Stop pressed just as the previous render finished would otherwise abort this one immediately.
    Renderer::stopRequested = false;
    traceFuture = std::async(std::launch::async, [&]() {
      image.clear();
      scene.cam.initialize();
      logger.log("Rendering... ", scene.cam.imageWidth, "x", scene.cam.imageHeight, " with ",
                 scene.cam.samplesPerPixel, " samples per pixel");
      auto startTime = std::chrono::high_resolution_clock::now();

      status = RENDERING;
      renderer->renderScene(scene, image, &progress);
      lastSpp = scene.cam.uncappedSpp ? renderer->film.getAverageSampleCount()
                                        : scene.cam.sqrtSpp * scene.cam.sqrtSpp;
      status = DONE;

      auto endTime = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration<float, std::chrono::seconds::period>(endTime - startTime).count();

      logger.log("Image generated in ", duration, " seconds");
    });
  }

  void reset() {
    status = IDLE;
    progress = 0;
    renderer.reset();
  }

  void setupScene(GLResources& GLResources, int currentScene, float lookFrom[3], float lookAt[3]) {
    // Ensure currentScene is within valid range
    if (currentScene < 0 || currentScene >= static_cast<int>(sceneRegistry().size())) {
      logger.log("Invalid scene index: ", currentScene);
      return;
    }

    const SceneEntry& entry = sceneRegistry()[currentScene];
    logger.log("Setting up scene ... ", currentScene, " ", entry.name);
    scene = entry.make();

    GLResources.renderWidth = scene.cam.imageWidth;
    GLResources.renderHeight = static_cast<int>(scene.cam.imageWidth / scene.cam.aspectRatio);
    image.initialize(GLResources.renderWidth, GLResources.renderHeight);

    auto vec3tofloat = [](const Vec3& v, float f[3]) {
      f[0] = static_cast<float>(v[0]);
      f[1] = static_cast<float>(v[1]);
      f[2] = static_cast<float>(v[2]);
    };
    vec3tofloat(scene.cam.lookfrom, lookFrom);
    vec3tofloat(scene.cam.lookat, lookAt);

    GLResources.setupFramebuffer();
  }

  void stopRendering() {
    if (traceFuture.has_value()) {
      Renderer::stopRequested = true;
      // traceFuture->wait();
      // traceFuture.reset();
    }
  }
};

}  // namespace glimpse::gui
