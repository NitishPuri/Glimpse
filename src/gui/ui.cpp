// ImGui panels, pinned to the responsive layout (layout.h). Right-drag in the output panel moves the camera with
// cheap 2-spp previews, and the user's quality is restored with one full render on release.

#include "ui.h"

#include <chrono>
#include <filesystem>
#include <format>
#include <ranges>
#include <vector>

#include "app.h"
#include "camera_controls.h"
#include "imgui/implot.h"
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

// The live error is measured a few times a second: each sample copies the film, cheap next to rendering it.
constexpr double kErrorSampleInterval = 0.25;
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
    // `renderer` is released once a render completes, so use the spp recorded when it finished.
    const int spp = raytracer.lastSpp.load();

    // Same folder as the CLI's renders; lossless PNG like the CLI default.
    std::string dir = "results";
    std::filesystem::create_directories(dir);

    const auto outfilePath = std::format("{}/{:%Y%m%d%H%M%S}_{}_samples_{}.png", dir, localNow(),
                                         sceneRegistry()[params.currentScene].name, spp);

    if (raytracer.image.write(outfilePath)) {
      logger.log("Image written to {}", outfilePath);
    } else {
      logger.log("Failed to write image to {}", outfilePath);
    }
  }

  // Elapsed time, average spp and throughput, from the progress counter (one count per camera sample).
  auto currentStats = [&] {
    RenderStats s;
    s.seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - raytracer.startTime).count();
    const double samples = static_cast<double>(raytracer.progress.load());
    s.spp = samples / (static_cast<double>(glRes.renderWidth) * glRes.renderHeight);
    s.samplesPerSecond = s.seconds > 0 ? samples / s.seconds : 0;
    return s;
  };

  if (raytracer.status == RayTracer::RENDERING) {
    const RenderStats stats = currentStats();
    lastStats = stats;
    if (raytracer.scene.cam.uncappedSpp) {
      ImGui::Text("Average SPP ... %.1f", stats.spp);
      ImGui::ProgressBar(-1.0f * (float)ImGui::GetTime(), ImVec2(0.0f, 0.0f), "Progress..");
    } else {
      // The stratified grid renders sqrtSpp^2 samples (e.g. 10 requested -> 9), so count against that.
      const int spp = raytracer.scene.cam.sqrtSpp * raytracer.scene.cam.sqrtSpp;
      ImGui::Text("Rendering...%d/%d", static_cast<int>(stats.spp), spp);
      ImGui::ProgressBar(static_cast<float>(stats.spp / spp), ImVec2(-1, 0), "Progress");
    }
    ImGui::Text("%.1f s   %.2f Msamples/s", stats.seconds, stats.samplesPerSecond / 1e6);

    if (raytracer.measuring && stats.seconds - lastErrorSample >= kErrorSampleInterval) {
      raytracer.errors.sample(raytracer.renderer->film, raytracer.progress.load(), stats.seconds);
      lastErrorSample = stats.seconds;
    }

    // Uploaded every frame while rendering; the workers write the Image concurrently (see the viewer backlog).
    glRes.updateFramebuffer(raytracer.image.data());

  } else if (raytracer.status == RayTracer::DONE) {
    // The finished film is the curve's last point. Its time is late by at most one frame.
    lastStats = currentStats();
    if (raytracer.measuring) raytracer.errors.sample(raytracer.renderer->film, raytracer.progress.load(), lastStats->seconds);
    lastErrorSample = -1;
    glRes.updateFramebuffer(raytracer.image.data());
    ImGui::Text("Done");
    raytracer.reset();
  } else {
    ImGui::Text("Idle");
    if (lastStats)
      ImGui::Text("Last: %.1f spp in %.2f s, %.2f Msamples/s", lastStats->spp, lastStats->seconds,
                  lastStats->samplesPerSecond / 1e6);
  }
}

void UIRenderer::errorUI(RayTracer& raytracer, GLResources& glRes) {
  if (!ImGui::CollapsingHeader("Error vs reference", ImGuiTreeNodeFlags_DefaultOpen)) return;

  const std::string reason = raytracer.errors.unavailableReason(raytracer.scene);
  if (!reason.empty()) {
    ImGui::TextWrapped("%s", reason.c_str());
    const auto& ref = raytracer.errors.ref();
    if (ref && raytracer.scene.cam.imageWidth != ref->width && raytracer.status == RayTracer::IDLE &&
        ImGui::Button("Match reference size")) {
      raytracer.setRenderWidth(glRes, ref->width);
      maybeRenderOnParamChange(raytracer);
    }
    return;
  }

  const auto& series = raytracer.errors.series();
  if (series.empty()) {
    ImGui::TextWrapped("Render to measure against the %dx%d reference.", raytracer.errors.ref()->width,
                       raytracer.errors.ref()->height);
    return;
  }
  const ErrorPoint& last = series.back();
  ImGui::Text("%.1f spp   RMSE %.4g   relMSE %.4g", last.spp, last.error.rmse, last.error.relMSE);
  if (last.error.nonFinite > 0)
    ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "%zu NaN/inf values skipped", last.error.nonFinite);
  ImGui::Checkbox("x axis: time", &errorVsTime);

  // Until every pixel has a sample, unrendered (black) pixels dominate the error: plot from 1 spp on.
  std::vector<double> xs, ys;
  for (const ErrorPoint& p : series) {
    if (p.spp < 1 || p.error.relMSE <= 0) continue;
    xs.push_back(errorVsTime ? p.seconds : p.spp);
    ys.push_back(p.error.relMSE);
  }
  if (ImPlot::BeginPlot("##error", ImVec2(-1, 220))) {
    ImPlot::SetupAxes(errorVsTime ? "seconds" : "spp", "relMSE", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
    ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);
    ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);
    if (!xs.empty()) ImPlot::PlotLine("relMSE", xs.data(), ys.data(), static_cast<int>(xs.size()));
    if (xs.size() > 1) {
      // An unbiased estimator's error falls as 1/N: a slope of -1 on these axes, drawn from the first point.
      const double gx[2] = {xs.front(), xs.back()};
      const double gy[2] = {ys.front(), ys.front() * xs.front() / xs.back()};
      ImPlot::PlotLine("1/x", gx, gy, 2);
    }
    ImPlot::EndPlot();
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
    for (const auto& [n, entry] : std::views::enumerate(sceneRegistry())) {
      const bool isSelected = (params.currentScene == n);
      if (ImGui::Selectable(entry.name, isSelected)) {
        params.currentScene = static_cast<int>(n);
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

  // Outside the disabled block: the plot stays interactive while rendering.
  errorUI(raytracer, glRes);

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
