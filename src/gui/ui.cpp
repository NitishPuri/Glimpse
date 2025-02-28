#include "ui.h"

#include <filesystem>

#include "app.h"

void UIRenderer::cameraUI(RayTracer& raytracer, GLResources& gl_res) {
  // Render parameters
  ImGui::Checkbox("Uncap SPP", &raytracer.scene.cam.uncapped_spp);
  if (ImGui::SliderInt("Samples per Pixel", &raytracer.scene.cam.samples_per_pixel, 1, 1000)) {
    maybeRenderOnParamChange(raytracer);
  }
  if (ImGui::SliderInt("Max Depth", &raytracer.scene.cam.max_depth, 1, 100)) {
    maybeRenderOnParamChange(raytracer);
  }

  // Camera parameters
  if (ImGui::SliderFloat("Field of View", &raytracer.scene.cam.vfov, 1.0f, 180.0f)) {
    maybeRenderOnParamChange(raytracer);
  }
  if (ImGui::SliderFloat("Defocus Angle", &raytracer.scene.cam.defocus_angle, 0.0f, 10.0f)) {
    maybeRenderOnParamChange(raytracer);
  }
  if (ImGui::SliderFloat("Focus Distance", &raytracer.scene.cam.focus_distance, 0.0f, 50.0f)) {
    maybeRenderOnParamChange(raytracer);
  }

  if (ImGui::SliderFloat3("Look From", params.lookFrom, -10.0f, 10.0f)) {
    raytracer.scene.cam.lookfrom = point3(params.lookFrom[0], params.lookFrom[1], params.lookFrom[2]);
    maybeRenderOnParamChange(raytracer);
  }
  if (ImGui::SliderFloat3("Look At", params.lookAt, -10.0f, 10.0f)) {
    raytracer.scene.cam.lookat = point3(params.lookAt[0], params.lookAt[1], params.lookAt[2]);
    maybeRenderOnParamChange(raytracer);
  }

  ImGui::Text("Camera Manip :: ");
  ImGui::SameLine();
  if (ImGui::RadioButton("None", params.camera_mode == NONE)) {
    params.camera_mode = CameraMode::NONE;
  }
  ImGui::SameLine();
  if (ImGui::RadioButton("Fly Mode", params.camera_mode == FLY)) {
    params.camera_mode = CameraMode::FLY;
  }
  ImGui::SameLine();
  if (ImGui::RadioButton("Orbit Mode", params.camera_mode == ORBIT)) {
    params.camera_mode = CameraMode::ORBIT;
  }
}

void UIRenderer::renderControl(RayTracer& raytracer, GLResources& gl_res) {
  if (ImGui::Button("Render")) {
    raytracer.renderSceneAsync();
  }

  if (ImGui::Button("Save Image")) {
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    auto spp = raytracer.scene.cam.uncapped_spp ? raytracer.renderer->film.get_average_sample_count()
                                                : raytracer.scene.cam.samples_per_pixel;

    // Ensure the results directory exists
    std::string dir = "./glimpse_results";
    std::filesystem::create_directory(dir);

    std::ostringstream ss;
    ss << dir << "/" << std::put_time(std::localtime(&now_time), "%Y%m%d%H%M%S") << "_"
       << Scene::SceneNames[params.current_scene] << "_samples_" << spp << ".jpg";
    auto outfile_path = ss.str();

    if (raytracer.image.write(outfile_path) != 0) {
      logger.log("Image written to ", outfile_path);
    } else {
      logger.log("Failed to write image to ", outfile_path);
    }
  }

  if (raytracer.status == RayTracer::RENDERING) {
    if (raytracer.scene.cam.uncapped_spp) {
      // TODO: Implement this, add more stats
      auto avg_spp = raytracer.renderer->film.get_average_sample_count();
      ImGui::Text("Average SPP ... %d", avg_spp);
      ImGui::ProgressBar(-1.0f * (float)ImGui::GetTime(), ImVec2(0.0f, 0.0f), "Progress..");
    } else {
      int totalPixels = gl_res.renderWidth * gl_res.renderHeight * raytracer.scene.cam.samples_per_pixel;
      int samples_done = raytracer.progress.load();
      ImGui::Text("Rendering...%d/%d", samples_done / (gl_res.renderWidth * gl_res.renderHeight),
                  raytracer.scene.cam.samples_per_pixel);
      float progress = float(samples_done) / float(totalPixels);
      ImGui::ProgressBar(progress, ImVec2(-1, 0), "Progress");
    }

    // TODO: Should we conmtrol how frequent this happens?
    gl_res.updateFramebuffer(raytracer.image.data);

  } else if (raytracer.status == RayTracer::DONE) {
    gl_res.updateFramebuffer(raytracer.image.data);
    ImGui::Text("Done");
    raytracer.reset();
  } else {
    ImGui::Text("Idle");
  }
}

void UIRenderer::renderUI(RayTracer& raytracer, GLResources& gl_res) {
  bool controlsEnabled = (raytracer.status == RayTracer::IDLE);
  ImGui::Begin("Control Panel");

  if (!controlsEnabled) {
    ImGui::BeginDisabled();
  }

  if (ImGui::BeginCombo("Scene", Scene::SceneNames[params.current_scene].c_str())) {
    for (int n = 0; n < Scene::SceneNames.size(); n++) {
      const bool is_selected = (params.current_scene == n);
      if (ImGui::Selectable(Scene::SceneNames[n].c_str(), is_selected)) {
        params.current_scene = n;
        raytracer.setupScene(gl_res, params.current_scene, params.lookFrom, params.lookAt);
        maybeRenderOnParamChange(raytracer);
      }
      if (is_selected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
  // Scene parameters
  if (ImGui::ColorEdit3("Background Color", params.backgroundColor)) {
    raytracer.scene.background = color(params.backgroundColor[0], params.backgroundColor[1], params.backgroundColor[2]);
    maybeRenderOnParamChange(raytracer);
  }

  // Camera parameters
  cameraUI(raytracer, gl_res);
  renderControl(raytracer, gl_res);

  if (!controlsEnabled) {
    ImGui::EndDisabled();

    if (raytracer.scene.cam.uncapped_spp) {
      if (ImGui::Button("Stop Rendering")) {
        raytracer.stopRendering();
      }
    }
  }

  ImGui::Checkbox("Auto Render", &params.auto_render);

  //   static bool plot_pdf = true;
  //   if (ImGui::Button("Plot PDF")) {
  //     plot_pdf = true;
  //   }

  ImGui::End();

  // if (plot_pdf) {
  //   plotPdf();
  //   //estimatePi();
  // }
}

void UIRenderer::renderOutput(GLResources& gl_res, RayTracer& raytracer) {
    ImGui::Begin("Render Output");
  // flip vertically
  ImGui::Image(ImTextureID(gl_res.framebufferTexture), calculatePanelSize(gl_res), ImVec2(0, 1), ImVec2(1, 0));

  // Check if the Rener Output window is hovered
  if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
    //  camera mode

    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
    ImGui::SetWindowFocus();
    ImGui::SetWindowPos(ImGui::GetWindowPos(), ImGuiCond_Always);

    ImVec2 mouse_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
    ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);

    float dx = mouse_delta.x * 0.005f;
    float dy = mouse_delta.y * 0.005f;

    if (params.camera_mode == FLY) {
      raytracer.scene.cam.fly(dx, dy);
      //
      raytracer.scene.cam.samples_per_pixel = 2;
      raytracer.scene.cam.max_depth = 2;
      maybeRenderOnParamChange(raytracer);
    } else if (params.camera_mode == ORBIT) {
      raytracer.scene.cam.orbit(dx, dy);
      raytracer.scene.cam.samples_per_pixel = 2;
      raytracer.scene.cam.max_depth = 2;
      maybeRenderOnParamChange(raytracer);
    }
  }

  ImGui::End();
}

ImVec2 UIRenderer::calculatePanelSize(GLResources& gl_res) {
  ImVec2 available_size = ImGui::GetContentRegionAvail();
  float aspect_ratio = static_cast<float>(gl_res.renderWidth) / gl_res.renderHeight;
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

void UIRenderer::maybeRenderOnParamChange(RayTracer& raytracer) {
  if (params.auto_render) {
    raytracer.renderSceneAsync();
  }
}
