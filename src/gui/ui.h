#pragma once

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

  Logger& logger;
};