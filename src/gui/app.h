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

#include "core/scenes.h"
#include "core/image.h"
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
      logger.log("Failed to create GLFW window");
      glfwTerminate();
      return -1;
    }

    glfwMakeContextCurrent(window);
    // glfwSwapInterval(1); //TODO Enable vsync
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    setupFramebuffer();
    setupRaytracer();

    return 0;
  }

  void run() {
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();

      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      renderUI();

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
    int current_scene = 2;
  } ImGuiParams;

  struct RayTracerSt {
    Scene scene;
    Image image;
    enum Status { IDLE, RENDERING, DONE };
    std::optional<std::future<void>> trace_future;
    std::atomic<Status> status = IDLE;
    std::atomic<int> progress = 0;
  } RayTracer;

  struct GlResources {
    // Texture
    GLuint framebufferTexture;
    int renderWidth = WINDOW_WIDTH / 2;
    int renderHeight = WINDOW_HEIGHT / 2;
  } Resources;

  GLFWwindow* window;
  Logger logger;

  void setupFramebuffer() {
    glGenTextures(1, &Resources.framebufferTexture);
    glBindTexture(GL_TEXTURE_2D, Resources.framebufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Resources.renderWidth,
                 Resources.renderHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  void setupRaytracer() {
    // TODO: use size from scene
    logger.log("Setting up scene ... ", ImGuiParams.current_scene,
               SceneNames[ImGuiParams.current_scene]);
    RayTracer.scene = SceneMap[SceneNames[ImGuiParams.current_scene]]();
    RayTracer.image.initialize(Resources.renderWidth, Resources.renderHeight);
  }

  void updateFramebuffer(const std::vector<uint8_t>& imageData) {
    glBindTexture(GL_TEXTURE_2D, Resources.framebufferTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Resources.renderWidth,
                    Resources.renderHeight, GL_RGB, GL_UNSIGNED_BYTE,
                    imageData.data());
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
          //   RayTracer.scene =
          //   SceneMap[SceneNames[ImGuiParams.current_scene]]();
        }
        if (is_selected) ImGui::SetItemDefaultFocus();
      }
      ImGui::EndCombo();
    }

    if (ImGui::Button("Render")) {
      RayTracer.trace_future = std::async(std::launch::async, [&]() {
        RayTracer.image.clear();
        logger.log("Rendering... ", Resources.renderWidth, "x",
                   Resources.renderHeight, " with ",
                   RayTracer.scene.samples_per_pixel, " samples per pixel");
        auto startTime = std::chrono::high_resolution_clock::now();

        RayTracer.status = RayTracerSt::RENDERING;
        render_scene(RayTracer.scene, RayTracer.image, &RayTracer.progress);
        RayTracer.status = RayTracerSt::DONE;

        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration =
            std::chrono::duration<float, std::chrono::seconds::period>(
                endTime - startTime)
                .count();

        logger.log("Image generated in ", duration, " seconds");
      });
    }
    if (RayTracer.status == RayTracerSt::RENDERING) {
      int totalPixels = Resources.renderWidth * Resources.renderHeight *
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

  void renderOutput() {
    ImGui::Begin("Render Output");

    ImVec2 available_size = ImGui::GetContentRegionAvail();
    float aspect_ratio =
        static_cast<float>(Resources.renderWidth) / Resources.renderHeight;
    ImVec2 image_size;

    if (available_size.x / aspect_ratio <= available_size.y) {
      image_size.x = available_size.x;
      image_size.y = available_size.x / aspect_ratio;
    } else {
      image_size.x = available_size.y * aspect_ratio;
      image_size.y = available_size.y;
    }

    ImGui::Image(ImTextureID(Resources.framebufferTexture), image_size);
    ImGui::End();
  }
};