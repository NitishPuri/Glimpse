#pragma once

#include <vector>

#include "glad/glad.h"
//
#include "GLFW/glfw3.h"
#include "core/logger.h"

struct GLResources {
  GLResources(Logger& logger) : logger(logger) {}
  GLuint framebufferTexture{};
  int renderWidth{};
  int renderHeight{};
  GLFWwindow* window = nullptr;

  Logger& logger;

  int initGL();

  void setupFramebuffer();
  bool checkGLError(const std::string& functionName);
  void updateFramebuffer(const std::vector<uint8_t>& imageData);
};