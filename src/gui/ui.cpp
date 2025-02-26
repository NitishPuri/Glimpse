#include "ui.h"

#include "app.h"

void cameraUI(RayTracer& raytracer, ImGuiParams& gui_params, Logger& logger, GLResources& gl_res) {
  // Render parameters
  if (ImGui::SliderInt("Samples per Pixel", &raytracer.scene.cam.samples_per_pixel, 1, 1000)) {
    raytracer.renderSceneAsync(logger, gl_res);
  }
  if (ImGui::SliderInt("Max Depth", &raytracer.scene.cam.max_depth, 1, 100)) {
    raytracer.renderSceneAsync(logger, gl_res);
  }

  // Camera parameters
  if (ImGui::SliderFloat("Field of View", &raytracer.scene.cam.vfov, 1.0f, 180.0f)) {
    raytracer.renderSceneAsync(logger, gl_res);
  }
  if (ImGui::SliderFloat("Aperture", &raytracer.scene.cam.aperture, 0.0f, 10.0f)) {
    raytracer.renderSceneAsync(logger, gl_res);
  }
  if (ImGui::SliderFloat("Focus Distance", &raytracer.scene.cam.focus_distance, 0.0f, 50.0f)) {
    raytracer.renderSceneAsync(logger, gl_res);
  }

  if (ImGui::SliderFloat3("Look From", gui_params.lookFrom, -10.0f, 10.0f)) {
    raytracer.scene.cam.lookfrom = point3(gui_params.lookFrom[0], gui_params.lookFrom[1], gui_params.lookFrom[2]);
    raytracer.renderSceneAsync(logger, gl_res);
  }
  if (ImGui::SliderFloat3("Look At", gui_params.lookAt, -10.0f, 10.0f)) {
    raytracer.scene.cam.lookat = point3(gui_params.lookAt[0], gui_params.lookAt[1], gui_params.lookAt[2]);
    raytracer.renderSceneAsync(logger, gl_res);
  }
}

void UIRenderer::renderUI(ImGuiParams& gui_params, RayTracer& raytracer, GLResources& gl_res, Logger& logger) {
  ImGui::Begin("Control Panel");

  if (ImGui::BeginCombo("Scene", Scene::SceneNames[gui_params.current_scene].c_str())) {
    for (int n = 0; n < Scene::SceneNames.size(); n++) {
      const bool is_selected = (gui_params.current_scene == n);
      if (ImGui::Selectable(Scene::SceneNames[n].c_str(), is_selected)) {
        gui_params.current_scene = n;
        raytracer.setupScene(logger, gl_res, gui_params.current_scene, gui_params.lookFrom, gui_params.lookAt);
        raytracer.renderSceneAsync(logger, gl_res);
      }
      if (is_selected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }

  // Camera parameters
  cameraUI(raytracer, gui_params, logger, gl_res);

  // Scene parameters
  if (ImGui::ColorEdit3("Background Color", gui_params.backgroundColor)) {
    raytracer.scene.background =
        color(gui_params.backgroundColor[0], gui_params.backgroundColor[1], gui_params.backgroundColor[2]);
    raytracer.renderSceneAsync(logger, gl_res);
  }

  if (ImGui::Button("Render")) {
    // Call renderSceneAsync from AppWindow
    raytracer.renderSceneAsync(logger, gl_res);
  }

  if (ImGui::Button("Save Image")) {
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);

    std::ostringstream ss;
    ss << "./results/" << std::put_time(std::localtime(&now_time), "%Y%m%d%H%M%S") << "_"
       << Scene::SceneNames[gui_params.current_scene] << ".jpg";
    auto outfile_path = ss.str();

    // raytracer.image.write(ss.str(), true);

    if (raytracer.image.write(outfile_path) != 0) {
      logger.log("Image written to ", outfile_path);
    } else {
      logger.log("Failed to write image to ", outfile_path);
    }
  }

  if (raytracer.status == RayTracer::RENDERING) {
    int totalPixels = gl_res.renderWidth * gl_res.renderHeight * raytracer.scene.cam.samples_per_pixel;
    ImGui::Text("Rendering...%d/%d", raytracer.progress.load(), totalPixels);
    float progress = float(raytracer.progress.load()) / float(totalPixels);
    ImGui::ProgressBar(progress, ImVec2(-1, 0), "Progress");
    // Call updateFramebuffer from AppWindow
    gl_res.updateFramebuffer(raytracer.image.data, logger);

  } else if (raytracer.status == RayTracer::DONE) {
    // Call updateFramebuffer from AppWindow
    gl_res.updateFramebuffer(raytracer.image.data, logger);
    ImGui::Text("Done");
    raytracer.status = RayTracer::IDLE;
    raytracer.progress = 0;
  } else {
    ImGui::Text("Idle");
  }

  ImGui::End();
}

void UIRenderer::renderOutput(GLResources& GLResources) {
  ImGui::Begin("Render Output");
  // flip vertically
  ImGui::Image(ImTextureID(GLResources.framebufferTexture), calculatePanelSize(GLResources), ImVec2(0, 1),
               ImVec2(1, 0));
  ImGui::End();
}

ImVec2 UIRenderer::calculatePanelSize(GLResources& GLResources) {
  ImVec2 available_size = ImGui::GetContentRegionAvail();
  float aspect_ratio = static_cast<float>(GLResources.renderWidth) / GLResources.renderHeight;
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