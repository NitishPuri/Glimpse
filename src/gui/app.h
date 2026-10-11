#pragma once

// AppWindow: owns the window/GL resources, the UI and the ray tracer, and runs the frame loop.

#include "config.h"
#include "glimpse/util/log.h"
#include "gl_res.h"
#include "raytracer.h"
#include "ui.h"

namespace glimpse::gui {

class AppWindow {
 public:
  AppWindow(Logger& logger_) : logger(logger_), ui(logger_), glRes(logger_), raytracer(logger_) {}

  int initApp();
  void run();

 private:
  Logger& logger;

  RayTracer raytracer;
  GLResources glRes;
  UIRenderer ui;
};

}  // namespace glimpse::gui
