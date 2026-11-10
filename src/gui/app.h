#pragma once

// AppWindow: owns the window/GL resources, the UI and the ray tracer, and runs the frame loop.

#include "config.h"
#include "gl_res.h"
#include "raytracer.h"
#include "ui.h"

namespace glimpse::gui {

class AppWindow {
 public:
  int initApp();
  void run();

 private:
  RayTracer raytracer;
  GLResources glRes;
  UIRenderer ui;
};

}  // namespace glimpse::gui
