#include "ui.h"

#include "app.h"

void UIRenderer::renderUI(ImGuiParams& ImGuiParams, RayTracer& RayTracer,
                          GLResources& GLResources, Logger& logger) {
  ImGui::Begin("Control Panel");

  if (ImGui::BeginCombo("Scene",
                        Scene::SceneNames[ImGuiParams.current_scene].c_str())) {
    for (int n = 0; n < Scene::SceneNames.size(); n++) {
      const bool is_selected = (ImGuiParams.current_scene == n);
      if (ImGui::Selectable(Scene::SceneNames[n].c_str(), is_selected)) {
        ImGuiParams.current_scene = n;
        if (window) window->setupScene();
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
  ImGui::SliderFloat("Field of View", &RayTracer.scene.vfov, 1.0f, 180.0f);
  ImGui::SliderFloat("Aperture", &RayTracer.scene.aperture, 0.0f, 10.0f);

  if (ImGui::SliderFloat3("Look From", ImGuiParams.lookFrom, -10.0f, 10.0f)) {
    RayTracer.scene.lookfrom =
        point3(ImGuiParams.lookFrom[0], ImGuiParams.lookFrom[1],
               ImGuiParams.lookFrom[2]);
    RayTracer.renderSceneAsync(logger, GLResources);
  }
  if (ImGui::SliderFloat3("Look At", ImGuiParams.lookAt, -10.0f, 10.0f)) {
    RayTracer.scene.lookat = point3(
        ImGuiParams.lookAt[0], ImGuiParams.lookAt[1], ImGuiParams.lookAt[2]);
    RayTracer.renderSceneAsync(logger, GLResources);
  }

  if (ImGui::Button("Render")) {
    // Call renderSceneAsync from AppWindow
    RayTracer.renderSceneAsync(logger, GLResources);
  }
  if (RayTracer.status == RayTracer::RENDERING) {
    int totalPixels = GLResources.renderWidth * GLResources.renderHeight *
                      RayTracer.scene.samples_per_pixel;
    ImGui::Text("Rendering...%d/%d", RayTracer.progress.load(), totalPixels);
    float progress = float(RayTracer.progress.load()) / float(totalPixels);
    ImGui::ProgressBar(progress, ImVec2(-1, 0), "Progress");
    // Call updateFramebuffer from AppWindow
    if (window) window->updateFramebuffer(window->RayTracer.image.data);

  } else if (RayTracer.status == RayTracer::DONE) {
    // Call updateFramebuffer from AppWindow
    if (window) window->updateFramebuffer(window->RayTracer.image.data);
    ImGui::Text("Done");
    RayTracer.status = RayTracer::IDLE;
    RayTracer.progress = 0;
  } else {
    ImGui::Text("Idle");
  }

  ImGui::End();
}

void UIRenderer::renderOutput(GLResources& GLResources) {
  ImGui::Begin("Render Output");
  // flip vertically
  ImGui::Image(ImTextureID(GLResources.framebufferTexture),
               calculatePanelSize(GLResources), ImVec2(0, 1), ImVec2(1, 0));
  ImGui::End();
}

ImVec2 UIRenderer::calculatePanelSize(GLResources& GLResources) {
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