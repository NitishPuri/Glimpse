#pragma once

#include "glad/glad.h"
//
#include "GLFW/glfw3.h"
#include "core/logger.h"

struct GLResources {
  GLuint framebufferTexture;
  int renderWidth;
  int renderHeight;

  void setupFramebuffer(Logger& logger) {
    glGenTextures(1, &framebufferTexture);
    if (checkGLError("glGenTextures", logger)) return;

    glBindTexture(GL_TEXTURE_2D, framebufferTexture);
    if (checkGLError("glBindTexture", logger)) return;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, renderWidth, renderHeight, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, nullptr);
    if (checkGLError("glTexImage2D", logger)) return;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    if (checkGLError("glTexParameteri MIN_FILTER", logger)) return;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (checkGLError("glTexParameteri MAG_FILTER", logger)) return;
  }

  bool checkGLError(const std::string& functionName, Logger& logger) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
      logger.log("OpenGL error in ", functionName, ": ", err);
      return true;
    }
    return false;
  }
};