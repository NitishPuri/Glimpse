#pragma once

#include "gl_res.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "raytracer.h"

struct ImGuiParams {
  int current_scene = 10;
  std::string syatyScene = "two_diffuse_spheres";
  float backgroundColor[3] = {0.1f, 0.1f, 0.1f};
  float lookFrom[3] = {13.0f, 2.0f, 3.0f};
  float lookAt[3] = {0.0f, 0.0f, 0.0f};
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