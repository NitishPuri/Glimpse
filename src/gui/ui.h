#pragma once

#include <optional>
#include <utility>

#include "config.h"
#include "gl_res.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "raytracer.h"

namespace glimpse::gui {

class AppWindow;

class UIRenderer {
 public:
  ImGuiParams params;
  UIRenderer(Logger& logger) : logger(logger) {}

  void renderUI(RayTracer& RayTracer, GLResources& GLResources);
  void renderOutput(GLResources& GLResources, RayTracer& raytracer);

 private:
  void maybeRenderOnParamChange(RayTracer& raytracer);
  void cameraUI(RayTracer& raytracer, GLResources& glRes);
  void renderControl(RayTracer& raytracer, GLResources& glRes);

  ImVec2 calculatePanelSize(GLResources& GLResources);

  // Camera drag: the user's {samplesPerPixel, maxDepth} while preview quality is in use.
  std::optional<std::pair<int, int>> dragSavedQuality;
  bool renderWhenIdle = false;

  Logger& logger;
};

}  // namespace glimpse::gui
