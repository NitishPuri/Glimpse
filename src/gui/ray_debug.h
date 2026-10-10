#if 0
#pragma once

#include <memory>
#include <vector>

#include "glimpse/core/camera.h"
#include "glimpse/core/scene.h"
#include "glimpse/debug/ray_analyzer.h"
#include "glimpse/debug/ray_visualizer.h"

namespace glimpse {
namespace debug {

class DebugUI {
 public:
  DebugUI(Scene& scene, Camera& camera);

  void render();
  void update();

  // Set the current pixel for analysis
  void setDebugPixel(int x, int y);

  // Set the region for analysis
  void setDebugRegion(int x0, int y0, int x1, int y1);

 private:
  Scene& m_scene;
  Camera& m_camera;
  RayVisualizer m_visualizer;
  RayAnalyzer m_analyzer;

  int m_debug_pixel_x = -1;
  int m_debug_pixel_y = -1;
  int m_debug_region_x0 = -1;
  int m_debug_region_y0 = -1;
  int m_debug_region_x1 = -1;
  int m_debug_region_y1 = -1;

  std::vector<PathMetrics> m_current_paths;
  std::unordered_map<std::string, float> m_current_stats;
  bool m_show_path_visualization = true;
  bool m_show_path_analysis = true;
  int m_debug_samples = 10;
  int m_current_path_idx = 0;

  void renderSingleRayDebugger();
  void renderRegionVisualizer();
  void renderPathAnalysis();
  void renderPathStatsPlot();
};

}  // namespace debug
}  // namespace glimpse

#endif