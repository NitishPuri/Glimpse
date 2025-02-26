
#include "gl_res.h"

void GLResources::setupFramebuffer(Logger& logger) {
  glGenTextures(1, &framebufferTexture);
  if (checkGLError("glGenTextures", logger)) return;

  glBindTexture(GL_TEXTURE_2D, framebufferTexture);
  if (checkGLError("glBindTexture", logger)) return;

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, renderWidth, renderHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
  if (checkGLError("glTexImage2D", logger)) return;

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  if (checkGLError("glTexParameteri MIN_FILTER", logger)) return;

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  if (checkGLError("glTexParameteri MAG_FILTER", logger)) return;
}

bool GLResources::checkGLError(const std::string& functionName, Logger& logger) {
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    logger.log("OpenGL error in ", functionName, ": ", err);
    return true;
  }
  return false;
}
void GLResources::updateFramebuffer(const std::vector<uint8_t>& imageData, Logger& logger) {
  glBindTexture(GL_TEXTURE_2D, framebufferTexture);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, renderWidth, renderHeight, GL_RGB, GL_UNSIGNED_BYTE, imageData.data());
  if (checkGLError("glTexSubImage2D", logger)) return;
}
