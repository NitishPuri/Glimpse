#pragma once

// GLResources: the GLFW window and the GL texture the rendered Image is uploaded to.

#include <vector>

#include "glad/glad.h"
//
#include "GLFW/glfw3.h"
#include "glimpse/util/log.h"

namespace glimpse::gui {

struct GLResources {
  GLuint framebufferTexture{};
  int renderWidth{};
  int renderHeight{};
  GLFWwindow* window = nullptr;

  int initGL();

  void setupFramebuffer();
  bool checkGLError(const std::string& functionName);
  void updateFramebuffer(const std::vector<uint8_t>& imageData);
};

}  // namespace glimpse::gui
