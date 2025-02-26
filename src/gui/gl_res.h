#pragma once

#include <vector>

#include "glad/glad.h"
//
#include "GLFW/glfw3.h"
#include "core/logger.h"

struct GLResources {
  GLuint framebufferTexture;
  int renderWidth;
  int renderHeight;
  // GLFWwindow* window;

  void setupFramebuffer(Logger& logger);
  bool checkGLError(const std::string& functionName, Logger& logger);
  void updateFramebuffer(const std::vector<uint8_t>& imageData, Logger& logger);
};