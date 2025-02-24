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

  int initApp() {
    if (!glfwInit()) {
      logger.log("Failed to initialize GLFW");
      return -1;
    }

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Glimpse", nullptr,
                              nullptr);
    if (!window) {
      const char* description;
      int code = glfwGetError(&description);
      logger.log("Failed to create GLFW window: ", description);
      glfwTerminate();
      return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // Enable vsync
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      logger.log("Failed to initialize GLAD");
      return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    if (glfwGetCurrentContext() == window) {
      setupRaytracer();
    } else {
      logger.log("Failed to initialize OpenGL context");
      return -1;
    }

    ui.setApp(this);

    return 0;
  }

  void run() {
    bool firstFrame = true;
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();

      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      ui.renderUI(ImGuiParams, RayTracer, GLResources, logger);

      if (firstFrame) {
        firstFrame = false;
        RayTracer.renderSceneAsync(logger, GLResources);
      }

      glClearColor(ImGuiParams.backgroundColor[0],
                   ImGuiParams.backgroundColor[1],
                   ImGuiParams.backgroundColor[2], 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      ui.renderOutput(GLResources);

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
  }

  RayTracer RayTracer;
  GLResources GLResources;
  ImGuiParams ImGuiParams;
  UIRenderer ui;

  GLFWwindow* window;
  Logger logger;

 public:
  void setupRaytracer() {
    // Ensure current_scene is within valid range
    if (ImGuiParams.current_scene < 0 ||
        ImGuiParams.current_scene >= Scene::SceneNames.size()) {
      logger.log("Invalid scene index: ", ImGuiParams.current_scene);
      return;
    }

    // TODO: use size from scene
    logger.log("Setting up scene ... ", ImGuiParams.current_scene, " ",
               Scene::SceneNames[ImGuiParams.current_scene]);
    RayTracer.scene =
        Scene::SceneMap[Scene::SceneNames[ImGuiParams.current_scene]]();

    GLResources.renderWidth = RayTracer.scene.image_width;
    GLResources.renderHeight = static_cast<int>(RayTracer.scene.image_width /
                                                RayTracer.scene.aspect_ratio);
    RayTracer.image.initialize(GLResources.renderWidth,
                               GLResources.renderHeight);

    GLResources.setupFramebuffer(logger);
  }

  void updateFramebuffer(const std::vector<uint8_t>& imageData) {
    glBindTexture(GL_TEXTURE_2D, GLResources.framebufferTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GLResources.renderWidth,
                    GLResources.renderHeight, GL_RGB, GL_UNSIGNED_BYTE,
                    imageData.data());
    if (GLResources.checkGLError("glTexSubImage2D", logger)) return;
  }
};