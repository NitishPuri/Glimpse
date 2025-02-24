#pragma once

#include "gl_res.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "raytracer.h"

struct ImGuiParams {
  float backgroundColor[3] = {0.1f, 0.1f, 0.1f};
  int current_scene;
};

class AppWindow;

class UIRenderer {
 public:
  void setApp(AppWindow* window) { this->window = window; }
  void renderUI(ImGuiParams& ImGuiParams, RayTracer& RayTracer,
                GLResources& GLResources, Logger& logger);
  void renderOutput(GLResources& GLResources);
  ImVec2 calculatePanelSize(GLResources& GLResources);

 private:
  AppWindow* window = nullptr;
};