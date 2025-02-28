#include "app.h"

int AppWindow::initApp() {
  if (gl_res.initGL() == -1) return -1;

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForOpenGL(gl_res.window, true);
  ImGui_ImplOpenGL3_Init("#version 330");

  // Setup Raytracer
  if (glfwGetCurrentContext() == gl_res.window) {
    if (!ui.params.startScene.empty()) {
      ui.params.current_scene =
          static_cast<int>(std::find(Scene::SceneNames.begin(), Scene::SceneNames.end(), ui.params.startScene) -
                           Scene::SceneNames.begin());
    }
    raytracer.setupScene(gl_res, ui.params.current_scene, ui.params.lookFrom, ui.params.lookAt);
  } else {
    logger.log("Failed to initialize OpenGL context");
    return -1;
  }

  return 0;
}

void AppWindow::run() {
  bool firstFrame = true;
  while (!glfwWindowShouldClose(gl_res.window)) {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ui.renderUI(raytracer, gl_res);

    if (firstFrame && ui.params.auto_render) {
      firstFrame = false;
      raytracer.renderSceneAsync();
    }

    glClearColor(.1f, .1f, .1f, 1.0f);  // Set background color to dark gray
    glClear(GL_COLOR_BUFFER_BIT);

    ui.renderOutput(gl_res, raytracer);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(gl_res.window);
  }

  glfwDestroyWindow(gl_res.window);
  glfwTerminate();
}

int main() {
  Logger logger(log_file_path);
  AppWindow app(logger);
  if (app.initApp() != 0) {
    return -1;
  }
  app.run();
  return 0;
}