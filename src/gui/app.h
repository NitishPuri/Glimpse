#pragma once

#include "core/logger.h"
#include "gl_res.h"
#include "raytracer.h"
#include "ui.h"

// GL Window dimensions
const int WINDOW_WIDTH = 1800;
const int WINDOW_HEIGHT = 1600;

const std::string log_file_path = "./log_gui.txt";

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

  GLFWwindow* window;
  Logger logger;
};