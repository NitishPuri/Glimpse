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
    // Initialize GLFW and OpenGL
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

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Setup Raytracer
    if (glfwGetCurrentContext() == window) {
      raytracer.setupScene(logger, gl_res, ui_params.current_scene,
                           ui_params.lookFrom, ui_params.lookAt);
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

      ui.renderUI(ui_params, raytracer, gl_res, logger);

      if (firstFrame) {
        firstFrame = false;
        raytracer.renderSceneAsync(logger, gl_res);
      }

      glClearColor(.1f, .1f, .1f, 1.0f);  // Set background color to dark gray
      glClear(GL_COLOR_BUFFER_BIT);

      ui.renderOutput(gl_res);

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
  }

  RayTracer raytracer;
  GLResources gl_res;
  ImGuiParams ui_params;
  UIRenderer ui;

  GLFWwindow* window;
  Logger logger;
};