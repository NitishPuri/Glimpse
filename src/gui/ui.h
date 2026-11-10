#pragma once

// UIRenderer: the Control Panel (scene, camera and sampling parameters, Render/Save/Stop, run stats, error vs the
// reference) and the Render Output panel.

#include <optional>
#include <utility>

#include "config.h"
#include "gl_res.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "raytracer.h"

namespace glimpse::gui {

class AppWindow;

class UIRenderer {
 public:
  ImGuiParams params;

  void renderUI(RayTracer& RayTracer, GLResources& GLResources);
  void renderOutput(GLResources& GLResources, RayTracer& raytracer);

 private:
  void maybeRenderOnParamChange(RayTracer& raytracer);
  void cameraUI(RayTracer& raytracer, GLResources& glRes);
  void renderControl(RayTracer& raytracer, GLResources& glRes);
  void errorUI(RayTracer& raytracer, GLResources& glRes);

  ImVec2 calculatePanelSize(GLResources& GLResources);

  // Camera drag: the user's {samplesPerPixel, maxDepth} while preview quality is in use.
  std::optional<std::pair<int, int>> dragSavedQuality;
  bool renderWhenIdle = false;

  // Run stats: the cost side of an equal-time comparison (technique 02).
  struct RenderStats {
    double seconds = 0;
    double spp = 0;  // average samples per pixel
    double samplesPerSecond = 0;
  };
  std::optional<RenderStats> lastStats;  // the running render's, or the last finished one's
  double lastErrorSample = -1;           // render time of the last ErrorTracker sample
  bool errorVsTime = false;              // plot x axis: seconds instead of spp
};

}  // namespace glimpse::gui
