#include "app.h"

int AppWindow::initApp() {
  if (gl_res.initGL(logger) == -1) return -1;

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForOpenGL(gl_res.window, true);
  ImGui_ImplOpenGL3_Init("#version 330");

  // Setup Raytracer
  if (glfwGetCurrentContext() == gl_res.window) {
    if (!ui_params.startScene.empty()) {
      ui_params.current_scene =
          static_cast<int>(std::find(Scene::SceneNames.begin(), Scene::SceneNames.end(), ui_params.startScene) -
                           Scene::SceneNames.begin());
    }
    raytracer.setupScene(logger, gl_res, ui_params.current_scene, ui_params.lookFrom, ui_params.lookAt);
  } else {
    logger.log("Failed to initialize OpenGL context");
    return -1;
  }

  ui.setApp(this);

  return 0;
}

void AppWindow::run() {
  bool firstFrame = true;
  while (!glfwWindowShouldClose(gl_res.window)) {
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

    glfwSwapBuffers(gl_res.window);
  }

  glfwDestroyWindow(gl_res.window);
  glfwTerminate();
}

int main() {
  AppWindow app;
  if (app.initApp() != 0) {
    return -1;
  }
  app.run();
  return 0;
}