#include "ui.h"

#include <filesystem>

#include "app.h"
#include "camera_controls.h"
#include "layout.h"

namespace glimpse::gui {

namespace {
// Pins the next ImGui window to a rect of the responsive layout (re-applied every frame, so it follows resizes).
void placeNextWindow(const glimpse::gui::Rect& r) {
  ImGui::SetNextWindowPos(ImVec2(r.x, r.y), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(r.w, r.h), ImGuiCond_Always);
}

glimpse::gui::Layout currentLayout() {
  const ImGuiViewport* vp = ImGui::GetMainViewport();
  return glimpse::gui::computeLayout(vp->WorkPos.x, vp->WorkPos.y, vp->WorkSize.x, vp->WorkSize.y);
}

constexpr ImGuiWindowFlags kPinned = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
}  // namespace

void UIRenderer::cameraUI(RayTracer& raytracer, GLResources& glRes) {
  // Render parameters
  ImGui::Checkbox("Uncap SPP", &raytracer.scene.cam.uncappedSpp);
  if (ImGui::SliderInt("Samples per Pixel", &raytracer.scene.cam.samplesPerPixel, 1, 1000)) {
    maybeRenderOnParamChange(raytracer);
  }
  if (ImGui::SliderInt("Max Depth", &raytracer.scene.cam.maxDepth, 1, 100)) {
    maybeRenderOnParamChange(raytracer);
  }

  // Camera parameters
  if (ImGui::SliderFloat("Field of View", &raytracer.scene.cam.vfov, 1.0f, 180.0f)) {
    maybeRenderOnParamChange(raytracer);
  }
  if (ImGui::SliderFloat("Defocus Angle", &raytracer.scene.cam.defocusAngle, 0.0f, 10.0f)) {
    maybeRenderOnParamChange(raytracer);
  }
  if (ImGui::SliderFloat("Focus Distance", &raytracer.scene.cam.focusDistance, 0.0f, 50.0f)) {
    maybeRenderOnParamChange(raytracer);
  }

  if (ImGui::SliderFloat3("Look From", params.lookFrom, -10.0f, 10.0f)) {
    raytracer.scene.cam.lookfrom = Point3(params.lookFrom[0], params.lookFrom[1], params.lookFrom[2]);
    maybeRenderOnParamChange(raytracer);
  }
  if (ImGui::SliderFloat3("Look At", params.lookAt, -10.0f, 10.0f)) {
    raytracer.scene.cam.lookat = Point3(params.lookAt[0], params.lookAt[1], params.lookAt[2]);
    maybeRenderOnParamChange(raytracer);
  }

  ImGui::Text("Camera Manip :: ");
  ImGui::SameLine();
  if (ImGui::RadioButton("None", params.cameraMode == NONE)) {
    params.cameraMode = CameraMode::NONE;
  }
  ImGui::SameLine();
  if (ImGui::RadioButton("Fly Mode", params.cameraMode == FLY)) {
    params.cameraMode = CameraMode::FLY;
  }
  ImGui::SameLine();
  if (ImGui::RadioButton("Orbit Mode", params.cameraMode == ORBIT)) {
    params.cameraMode = CameraMode::ORBIT;
  }
}

void UIRenderer::renderControl(RayTracer& raytracer, GLResources& glRes) {
  if (ImGui::Button("Render")) {
    raytracer.renderSceneAsync();
  }

  if (ImGui::Button("Save Image")) {
    auto now = std::chrono::system_clock::now();
    auto nowTime = std::chrono::system_clock::to_time_t(now);
    // `renderer` is released once a render completes, so use the spp recorded when it finished.
    const int spp = raytracer.lastSpp.load();

    // Same folder as the CLI's renders; lossless PNG like the CLI default.
    std::string dir = "results";
    std::filesystem::create_directories(dir);

    std::ostringstream ss;
    ss << dir << "/" << std::put_time(std::localtime(&nowTime), "%Y%m%d%H%M%S") << "_"
       << sceneRegistry()[params.currentScene].name << "_samples_" << spp << ".png";
    auto outfilePath = ss.str();

    if (raytracer.image.write(outfilePath)) {
      logger.log("Image written to ", outfilePath);
    } else {
      logger.log("Failed to write image to ", outfilePath);
    }
  }

  if (raytracer.status == RayTracer::RENDERING) {
    if (raytracer.scene.cam.uncappedSpp) {
      auto avgSpp = raytracer.renderer->film.getAverageSampleCount();
      ImGui::Text("Average SPP ... %d", avgSpp);
      ImGui::ProgressBar(-1.0f * (float)ImGui::GetTime(), ImVec2(0.0f, 0.0f), "Progress..");
    } else {
      // The stratified grid renders sqrtSpp^2 samples (e.g. 10 requested -> 9), so count against that.
      const int spp = raytracer.scene.cam.sqrtSpp * raytracer.scene.cam.sqrtSpp;
      const int pixels = glRes.renderWidth * glRes.renderHeight;
      const int samplesDone = raytracer.progress.load();
      ImGui::Text("Rendering...%d/%d", samplesDone / pixels, spp);
      float progress = float(samplesDone) / float(pixels * spp);
      ImGui::ProgressBar(progress, ImVec2(-1, 0), "Progress");
    }

    // Uploaded every frame while rendering; the workers write the Image concurrently (see the viewer backlog).
    glRes.updateFramebuffer(raytracer.image.data());

  } else if (raytracer.status == RayTracer::DONE) {
    glRes.updateFramebuffer(raytracer.image.data());
    ImGui::Text("Done");
    raytracer.reset();
  } else {
    ImGui::Text("Idle");
  }
}

void UIRenderer::renderUI(RayTracer& raytracer, GLResources& glRes) {
  bool controlsEnabled = (raytracer.status == RayTracer::IDLE);
  placeNextWindow(currentLayout().controls);
  ImGui::Begin("Control Panel", nullptr, kPinned);

  if (!controlsEnabled) {
    ImGui::BeginDisabled();
  }

  if (ImGui::BeginCombo("Scene", sceneRegistry()[params.currentScene].name)) {
    for (int n = 0; n < static_cast<int>(sceneRegistry().size()); n++) {
      const bool isSelected = (params.currentScene == n);
      if (ImGui::Selectable(sceneRegistry()[n].name, isSelected)) {
        params.currentScene = n;
        raytracer.setupScene(glRes, params.currentScene, params.lookFrom, params.lookAt);
        maybeRenderOnParamChange(raytracer);
      }
      if (isSelected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
  // Scene parameters
  if (ImGui::ColorEdit3("Background Color", params.backgroundColor)) {
    raytracer.scene.background = Color(params.backgroundColor[0], params.backgroundColor[1], params.backgroundColor[2]);
    maybeRenderOnParamChange(raytracer);
  }

  // Camera parameters
  cameraUI(raytracer, glRes);
  renderControl(raytracer, glRes);

  if (!controlsEnabled) {
    ImGui::EndDisabled();

    if (ImGui::Button("Stop Rendering")) {
      raytracer.stopRendering();
    }
  }

  ImGui::Checkbox("Auto Render", &params.autoRender);

  ImGui::End();
}

void UIRenderer::renderOutput(GLResources& glRes, RayTracer& raytracer) {
  placeNextWindow(currentLayout().output);
  ImGui::Begin("Render Output", nullptr, kPinned);
  // Image rows are top-down and so is ImGui's default uv (0,0) -> (1,1): no flip needed.
  ImGui::Image(ImTextureID(glRes.framebufferTexture), calculatePanelSize(glRes));

  // Camera drag: cheap preview renders (2 spp, depth 2) while dragging keep it interactive. On release the
  // user's quality comes back and one full render follows.
  auto& cam = raytracer.scene.cam;
  const bool dragging = params.cameraMode != NONE && ImGui::IsWindowHovered() &&
                        ImGui::IsMouseDragging(ImGuiMouseButton_Right);
  if (dragging) {
    if (!dragSavedQuality) dragSavedQuality = {cam.samplesPerPixel, cam.maxDepth};

    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
    ImGui::SetWindowFocus();

    ImVec2 mouseDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
    ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
    float dx = mouseDelta.x * 0.005f;
    float dy = mouseDelta.y * 0.005f;

    if (params.cameraMode == FLY) {
      fly(cam, dx, dy);
    } else {
      orbit(cam, dx, dy);
    }
    cam.samplesPerPixel = 2;
    cam.maxDepth = 2;
    maybeRenderOnParamChange(raytracer);
  } else if (dragSavedQuality && !ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
    cam.samplesPerPixel = dragSavedQuality->first;
    cam.maxDepth = dragSavedQuality->second;
    dragSavedQuality.reset();
    // The last preview may still be running (renderSceneAsync ignores requests while busy): render once idle.
    renderWhenIdle = params.autoRender;
  }
  if (renderWhenIdle && raytracer.status == RayTracer::IDLE) {
    renderWhenIdle = false;
    raytracer.renderSceneAsync();
  }

  ImGui::End();
}

ImVec2 UIRenderer::calculatePanelSize(GLResources& glRes) {
  ImVec2 availableSize = ImGui::GetContentRegionAvail();
  float aspectRatio = static_cast<float>(glRes.renderWidth) / glRes.renderHeight;
  ImVec2 imageSize;

  if (availableSize.x / aspectRatio <= availableSize.y) {
    imageSize.x = availableSize.x;
    imageSize.y = availableSize.x / aspectRatio;
  } else {
    imageSize.x = availableSize.y * aspectRatio;
    imageSize.y = availableSize.y;
  }

  return imageSize;
}

void UIRenderer::maybeRenderOnParamChange(RayTracer& raytracer) {
  if (params.autoRender) {
    raytracer.renderSceneAsync();
  }
}

}  // namespace glimpse::gui
