#include "app.h"

namespace glimpse::gui {

int AppWindow::initApp() {
  if (glRes.initGL() == -1) return -1;

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  // The layout is computed every frame (layout.h); a saved imgui.ini would only fight it.
  ImGui::GetIO().IniFilename = nullptr;
  ImGui_ImplGlfw_InitForOpenGL(glRes.window, true);
  ImGui_ImplOpenGL3_Init("#version 330");

  // Setup Raytracer
  if (glfwGetCurrentContext() == glRes.window) {
    // An unknown start scene keeps the default index (it used to become size(), one past the end).
    if (const SceneEntry* start = findScene(ui.params.startScene)) {
      ui.params.currentScene = static_cast<int>(start - sceneRegistry().data());
    }
    raytracer.setupScene(glRes, ui.params.currentScene, ui.params.lookFrom, ui.params.lookAt);
  } else {
    logger.log("Failed to initialize OpenGL context");
    return -1;
  }

  return 0;
}

void AppWindow::run() {
  bool firstFrame = true;
  while (!glfwWindowShouldClose(glRes.window)) {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ui.renderUI(raytracer, glRes);

    if (firstFrame && ui.params.autoRender) {
      firstFrame = false;
      raytracer.renderSceneAsync();
    }

    glClearColor(.1f, .1f, .1f, 1.0f);  // Set background color to dark gray
    glClear(GL_COLOR_BUFFER_BIT);

    ui.renderOutput(glRes, raytracer);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(glRes.window);
  }

  // A render still running would block forever in the std::async future's destructor (uncapped renders never
  // finish on their own). Ask it to stop and wait for the worker threads before tearing down the window.
  raytracer.stopRendering();
  if (raytracer.traceFuture) raytracer.traceFuture->wait();

  glfwDestroyWindow(glRes.window);
  glfwTerminate();
}

}  // namespace glimpse::gui


int main() {
  glimpse::Logger logger(glimpse::logFileFor("gui"));
  glimpse::gui::AppWindow app(logger);
  if (app.initApp() != 0) {
    return -1;
  }
  app.run();
  return 0;
}
