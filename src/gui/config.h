#pragma once

#include <string>

// GL Window dimensions
const int WINDOW_WIDTH = 1800;
const int WINDOW_HEIGHT = 1600;

const std::string log_file_path = "./log_gui.txt";

enum CameraMode { NONE, FLY, ORBIT };

struct ImGuiParams {
  // Startup options
  // std::string startScene = "material_showcase";
  // std::string startScene = "cornell_box";
  std::string startScene = "untitled_1";

  bool auto_render = true;

  CameraMode camera_mode = NONE;

  // scene parameters
  int current_scene = 1;
  float backgroundColor[3] = {0.1f, 0.1f, 0.1f};
  float lookFrom[3] = {13.0f, 2.0f, 3.0f};
  float lookAt[3] = {0.0f, 0.0f, 0.0f};
};
