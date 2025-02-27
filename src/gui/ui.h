#pragma once

#include "gl_res.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "raytracer.h"

struct ImGuiParams {
  int current_scene = 1;

  // std::string startScene = "material_showcase";
  std::string startScene = "cornell_box";

  bool auto_render = true;

  float backgroundColor[3] = {0.1f, 0.1f, 0.1f};
  float lookFrom[3] = {13.0f, 2.0f, 3.0f};
  float lookAt[3] = {0.0f, 0.0f, 0.0f};
};

class AppWindow;

class UIRenderer {
 public:
  ImGuiParams params;
  UIRenderer(Logger& logger) : logger(logger) {}

  void renderUI(RayTracer& RayTracer, GLResources& GLResources);
  void renderOutput(GLResources& GLResources);
  ImVec2 calculatePanelSize(GLResources& GLResources);

 private:
  void maybeRenderOnParamChange(RayTracer& raytracer) {
    if (params.auto_render) {
      raytracer.renderSceneAsync();
    }
  }
  void cameraUI(RayTracer& raytracer, GLResources& gl_res);
  void renderControl(RayTracer& raytracer, GLResources& gl_res);

  Logger& logger;
};