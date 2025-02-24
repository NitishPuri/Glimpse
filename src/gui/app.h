#pragma once

// clang-format off
#include <iostream>
#include <vector>
#include <optional>
#include <atomic>
#include <future>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "core/glimpse.h"
#include "core/render.h"
#include "core/logger.h"

// clang-format on

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

    return 0;
  }

  void run() {
    bool firstFrame = true;
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();

      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      renderUI();

      if (firstFrame) {
        firstFrame = false;
        renderSceneAsync();
      }

      glClearColor(ImGuiParams.backgroundColor[0],
                   ImGuiParams.backgroundColor[1],
                   ImGuiParams.backgroundColor[2], 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      renderOutput();

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
  }

 private:
  struct {
    float backgroundColor[3] = {0.1f, 0.1f, 0.1f};
    int current_scene = 0;
  } ImGuiParams;

  struct RayTracerSt {
    Scene scene;
    Image image;
    enum Status { IDLE, RENDERING, DONE };
    std::optional<std::future<void>> trace_future;
    std::atomic<Status> status = IDLE;
    std::atomic<int> progress = 0;
  } RayTracer;

  struct {
    // Texture
    GLuint framebufferTexture;
    int renderWidth = WINDOW_WIDTH / 2;
    int renderHeight = WINDOW_HEIGHT / 2;
  } GLResources;

  GLFWwindow* window;
  Logger logger;

  void setupFramebuffer() {
    glGenTextures(1, &GLResources.framebufferTexture);
    if (checkGLError("glGenTextures")) return;

    glBindTexture(GL_TEXTURE_2D, GLResources.framebufferTexture);
    if (checkGLError("glBindTexture")) return;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GLResources.renderWidth,
                 GLResources.renderHeight, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 nullptr);
    if (checkGLError("glTexImage2D")) return;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    if (checkGLError("glTexParameteri MIN_FILTER")) return;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (checkGLError("glTexParameteri MAG_FILTER")) return;
  }

  bool checkGLError(const std::string& functionName) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
      logger.log("OpenGL error in ", functionName, ": ", err);
      return true;
    }
    return false;
  }

  void setupRaytracer() {
    // Ensure current_scene is within valid range
    if (ImGuiParams.current_scene < 0 ||
        ImGuiParams.current_scene >= SceneNames.size()) {
      logger.log("Invalid scene index: ", ImGuiParams.current_scene);
      return;
    }

    // TODO: use size from scene
    logger.log("Setting up scene ... ", ImGuiParams.current_scene,
               SceneNames[ImGuiParams.current_scene]);
    RayTracer.scene = SceneMap[SceneNames[ImGuiParams.current_scene]]();

    GLResources.renderWidth = RayTracer.scene.image_width;
    GLResources.renderHeight = static_cast<int>(RayTracer.scene.image_width /
                                                RayTracer.scene.aspect_ratio);
    RayTracer.image.initialize(GLResources.renderWidth,
                               GLResources.renderHeight);

    setupFramebuffer();
  }

  void updateFramebuffer(const std::vector<uint8_t>& imageData) {
    glBindTexture(GL_TEXTURE_2D, GLResources.framebufferTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GLResources.renderWidth,
                    GLResources.renderHeight, GL_RGB, GL_UNSIGNED_BYTE,
                    imageData.data());
    if (checkGLError("glTexSubImage2D")) return;
  }

  void renderSceneAsync() {
    RayTracer.trace_future = std::async(std::launch::async, [&]() {
      RayTracer.image.clear();
      logger.log("Rendering... ", GLResources.renderWidth, "x",
                 GLResources.renderHeight, " with ",
                 RayTracer.scene.samples_per_pixel, " samples per pixel");
      auto startTime = std::chrono::high_resolution_clock::now();

      RayTracer.status = RayTracerSt::RENDERING;
      render_scene(RayTracer.scene, RayTracer.image, &RayTracer.progress);
      RayTracer.status = RayTracerSt::DONE;

      auto endTime = std::chrono::high_resolution_clock::now();
      auto duration =
          std::chrono::duration<float, std::chrono::seconds::period>(endTime -
                                                                     startTime)
              .count();

      logger.log("Image generated in ", duration, " seconds");
    });
  }

  void renderUI() {
    ImGui::Begin("Control Panel");

    if (ImGui::BeginCombo("Scene",
                          SceneNames[ImGuiParams.current_scene].c_str())) {
      for (int n = 0; n < SceneNames.size(); n++) {
        const bool is_selected = (ImGuiParams.current_scene == n);
        if (ImGui::Selectable(SceneNames[n].c_str(), is_selected)) {
          ImGuiParams.current_scene = n;
          setupRaytracer();
        }
        if (is_selected) ImGui::SetItemDefaultFocus();
      }
      ImGui::EndCombo();
    }

    // Render parameters
    ImGui::SliderInt("Samples per Pixel", &RayTracer.scene.samples_per_pixel, 1,
                     1000);
    ImGui::SliderInt("Max Depth", &RayTracer.scene.max_depth, 1, 100);

    // Camera parameters
    // ImGui::SliderFloat3("Camera Position",
    // &RayTracer.scene.camera.position[0],
    //                     -1000.0f, 1000.0f);
    // ImGui::SliderFloat3("Camera Target", &RayTracer.scene.camera.target[0],
    //                     -1000.0f, 1000.0f);
    ImGui::SliderFloat("Field of View", &RayTracer.scene.vfov, 1.0f, 180.0f);
    ImGui::SliderFloat("Aperture", &RayTracer.scene.aperture, 0.0f, 10.0f);
    // ImGui::SliderFloat("Focus Distance", &RayTracer.scene.camera.focus_dist,
    //                    0.1f, 1000.0f);

    if (ImGui::Button("Render")) {
      renderSceneAsync();
    }
    if (RayTracer.status == RayTracerSt::RENDERING) {
      int totalPixels = GLResources.renderWidth * GLResources.renderHeight *
                        RayTracer.scene.samples_per_pixel;
      ImGui::Text("Rendering...%d/%d", RayTracer.progress.load(), totalPixels);
      float progress = float(RayTracer.progress.load()) / float(totalPixels);
      ImGui::ProgressBar(progress, ImVec2(-1, 0), "Progress");
      updateFramebuffer(RayTracer.image.data);

    } else if (RayTracer.status == RayTracerSt::DONE) {
      updateFramebuffer(RayTracer.image.data);
      ImGui::Text("Done");
      RayTracer.status = RayTracerSt::IDLE;
      RayTracer.progress = 0;
    } else {
      ImGui::Text("Idle");
    }

    ImGui::End();
  }

  auto calculatePanelSize() {
    ImVec2 available_size = ImGui::GetContentRegionAvail();
    float aspect_ratio =
        static_cast<float>(GLResources.renderWidth) / GLResources.renderHeight;
    ImVec2 image_size;

    if (available_size.x / aspect_ratio <= available_size.y) {
      image_size.x = available_size.x;
      image_size.y = available_size.x / aspect_ratio;
    } else {
      image_size.x = available_size.y * aspect_ratio;
      image_size.y = available_size.y;
    }

    return image_size;
  }

  void renderOutput() {
    ImGui::Begin("Render Output");
    ImGui::Image(ImTextureID(GLResources.framebufferTexture),
                 calculatePanelSize());
    ImGui::End();
  }
};