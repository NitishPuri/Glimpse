
#include "gl_res.h"

#include "config.h"
#include "layout.h"

int GLResources::initGL() {
  // Initialize GLFW and OpenGL
  if (!glfwInit()) {
    logger.log("Failed to initialize GLFW");
    return -1;
  }

  // Open at 80% of the monitor's work area, centered; the window stays resizable and the layout follows it.
  int work_x = 0, work_y = 0, work_w = 0, work_h = 0;
  if (GLFWmonitor* monitor = glfwGetPrimaryMonitor()) glfwGetMonitorWorkarea(monitor, &work_x, &work_y, &work_w, &work_h);
  const auto size = glimpse::gui::initial_window_size(work_w, work_h, WINDOW_WIDTH, WINDOW_HEIGHT);

  window = glfwCreateWindow(size.w, size.h, "Glimpse", nullptr, nullptr);
  if (!window) {
    const char* description;
    int code = glfwGetError(&description);
    logger.log("Failed to create GLFW window: ", description, code);
    glfwTerminate();
    return -1;
  }
  if (work_w > 0) glfwSetWindowPos(window, work_x + (work_w - size.w) / 2, work_y + (work_h - size.h) / 2);

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);  // Enable vsync
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    logger.log("Failed to initialize GLAD");
    return -1;
  }

  return 0;
}

void GLResources::setupFramebuffer() {
  glGenTextures(1, &framebufferTexture);
  if (checkGLError("glGenTextures")) return;

  glBindTexture(GL_TEXTURE_2D, framebufferTexture);
  if (checkGLError("glBindTexture")) return;

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, renderWidth, renderHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
  if (checkGLError("glTexImage2D")) return;

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  if (checkGLError("glTexParameteri MIN_FILTER")) return;

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  if (checkGLError("glTexParameteri MAG_FILTER")) return;
}

bool GLResources::checkGLError(const std::string& functionName) {
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    logger.log("OpenGL error in ", functionName, ": ", err);
    return true;
  }
  return false;
}
void GLResources::updateFramebuffer(const std::vector<uint8_t>& imageData) {
  glBindTexture(GL_TEXTURE_2D, framebufferTexture);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, renderWidth, renderHeight, GL_RGB, GL_UNSIGNED_BYTE, imageData.data());
  if (checkGLError("glTexSubImage2D")) return;
}
