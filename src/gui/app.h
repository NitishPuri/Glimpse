#pragma once

#include "config.h"
#include "core/logger.h"
#include "gl_res.h"
#include "raytracer.h"
#include "ui.h"

class AppWindow {
 public:
  AppWindow() : logger(log_file_path) {}

  int initApp();
  void run();

 private:
  RayTracer raytracer;
  GLResources gl_res;
  ImGuiParams ui_params;
  UIRenderer ui;

  Logger logger;
};