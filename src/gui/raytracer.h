#pragma once

// RayTracer: runs Renderer::renderScene on a background thread (std::async) and exposes status/progress to the UI.
// lastSpp records the finished render's samples per pixel, because the renderer is released when it completes.
// `errors` measures the live film against the scene's reference (technique 02); the UI samples it while rendering.

#include <atomic>
#include <chrono>
#include <future>
#include <optional>

#include "glimpse/shapes.h"
#include "glimpse/util/image.h"
#include "glimpse/render.h"
#include "glimpse/scenes/registry.h"
#include "error_tracker.h"

namespace glimpse::gui {

struct RayTracer {
  Scene scene;
  Image image;
  enum Status { IDLE, RENDERING, DONE };
  std::atomic<Status> status{IDLE};
  std::optional<std::future<void>> traceFuture{};
  std::atomic<int64_t> progress{0};  // samples traced; 64-bit: an uncapped 600x600 render passes 2^31 in minutes
  // Samples per pixel of the last finished render (for the saved file name). Kept here because `renderer`, and
  // with it the film's sample counts, is released when a render completes.
  std::atomic<int> lastSpp{0};

  std::shared_ptr<Renderer> renderer;

  ErrorTracker errors;
  bool measuring = false;  // the current render is the reference's image, so its error is sampled
  std::chrono::steady_clock::time_point startTime;

  Logger& logger;
  RayTracer(Logger& logger) : logger(logger) {}

  void renderSceneAsync() {
    // check if we are already rendering, if yes, then lets do nothing for now.
    if (renderer.get() != nullptr) {
      return;
    }

    renderer = std::make_shared<Renderer>();
    startTime = std::chrono::steady_clock::now();
    // Decided once per render: camera drags edit the camera while a preview runs.
    errors.start();
    measuring = errors.unavailableReason(scene).empty();
    // A Stop pressed just as the previous render finished would otherwise abort this one immediately.
    Renderer::stopRequested = false;
    traceFuture = std::async(std::launch::async, [&]() {
      image.clear();
      scene.cam.initialize();
      logger.log("Rendering... {}x{} with {} samples per pixel", scene.cam.imageWidth, scene.cam.imageHeight,
                 scene.cam.samplesPerPixel);
      auto startTime = std::chrono::high_resolution_clock::now();

      status = RENDERING;
      renderer->renderScene(scene, image, &progress);
      lastSpp = scene.cam.uncappedSpp ? renderer->film.getAverageSampleCount()
                                        : scene.cam.sqrtSpp * scene.cam.sqrtSpp;
      status = DONE;

      auto endTime = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration<float, std::chrono::seconds::period>(endTime - startTime).count();

      logger.log("Image generated in {:.3f} seconds", duration);
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
      logger.log("Invalid scene index: {}", currentScene);
      return;
    }

    const SceneEntry& entry = sceneRegistry()[currentScene];
    logger.log("Setting up scene ... {} {}", currentScene, entry.name);
    scene = entry.make();
    errors.loadReference(entry.name, scene);

    auto vec3tofloat = [](const Vec3& v, float f[3]) {
      f[0] = static_cast<float>(v[0]);
      f[1] = static_cast<float>(v[1]);
      f[2] = static_cast<float>(v[2]);
    };
    vec3tofloat(scene.cam.lookfrom, lookFrom);
    vec3tofloat(scene.cam.lookat, lookAt);

    resizeOutput(GLResources);
  }

  // Changes the render width (height follows the aspect ratio), e.g. to match the reference's size.
  void setRenderWidth(GLResources& glRes, int width) {
    scene.cam.imageWidth = width;
    resizeOutput(glRes);
  }

  // Sizes the Image and the GL texture to the scene camera.
  void resizeOutput(GLResources& glRes) {
    glRes.renderWidth = scene.cam.imageWidth;
    glRes.renderHeight = static_cast<int>(scene.cam.imageWidth / scene.cam.aspectRatio);
    image.initialize(glRes.renderWidth, glRes.renderHeight);
    glRes.setupFramebuffer();
  }

  void stopRendering() {
    if (traceFuture.has_value()) {
      Renderer::stopRequested = true;
    }
  }
};

}  // namespace glimpse::gui
