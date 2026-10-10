#pragma once

#include <optional>
#include <utility>

#include "config.h"
#include "gl_res.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "raytracer.h"

class AppWindow;

class UIRenderer {
 public:
  ImGuiParams params;
  UIRenderer(Logger& logger) : logger(logger) {}

  void renderUI(RayTracer& RayTracer, GLResources& GLResources);
  void renderOutput(GLResources& GLResources, RayTracer& raytracer);

 private:
  void maybeRenderOnParamChange(RayTracer& raytracer);
  void cameraUI(RayTracer& raytracer, GLResources& gl_res);
  void renderControl(RayTracer& raytracer, GLResources& gl_res);

  ImVec2 calculatePanelSize(GLResources& GLResources);

  // Camera drag: the user's {samples_per_pixel, max_depth} while preview quality is in use.
  std::optional<std::pair<int, int>> drag_saved_quality;
  bool render_when_idle = false;

  Logger& logger;
};