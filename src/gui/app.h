#pragma once

#include "config.h"
#include "core/logger.h"
#include "gl_res.h"
#include "raytracer.h"
#include "ui.h"

class AppWindow {
 public:
  AppWindow(Logger& logger_) : logger(logger_), ui(logger_), gl_res(logger_), raytracer(logger_) {}

  int initApp();
  void run();

 private:
  Logger& logger;

  RayTracer raytracer;
  GLResources gl_res;
  UIRenderer ui;
};