#if 0
#pragma once

#include <memory>
#include <vector>

// #include "glm/glm.hpp"
// #include "hit_record.h"
#include "ray.h"
#include "scenes.h"

// namespace glimpse {
// namespace debug {

struct PathSegment {
  vec3 start;
  vec3 end;
  vec3 color;  // Could represent ray type (reflection, refraction, etc.)
  int depth;
  std::string event_type;  // "reflection", "refraction", "absorption", etc.
  float importance;        // For visualizing path contribution
};

class RayVisualizer {
 public:
  // Track a single ray path through the scene
  void trackRay(const ray& ray, const Scene& scene, int max_depth = 5);

  // Track rays for a region of pixels
  void trackRegion(int x0, int y0, int x1, int y1, const Scene& scene, int max_depth = 5);

  // Visualization options
  //   void drawPaths(GLFWwindow* window, const Camera& view_camera);

  // Get path data for ImGui display
  const std::vector<PathSegment>& getPathSegments() const { return m_path_segments; }

  // Clear tracked paths
  void clear() { m_path_segments.clear(); }

 private:
  std::vector<PathSegment> m_path_segments;

  // Helper to sample a ray and track its path
  void tracePath(const ray& ray, const Scene& scene, int max_depth);
};

////////////////////////////////////////

struct RayEvent {
  enum Type { CREATED, HIT, SCATTERED, ABSORBED, MISSED };

  Type type;
  int depth;
  vec3 position;
  vec3 direction;
  vec3 normal;       // For HIT events
  vec3 attenuation;  // For SCATTERED events
  std::string material_name;
  float contribution;  // How much this ray contributed to the final color
  float pdf;           // Probability density function if applicable
};

struct PathMetrics {
  float total_contribution;
  float variance;
  int bounces;
  int diffuse_bounces;
  int specular_bounces;
  int transmission_bounces;
  float path_length;
  float avg_cos_theta;  // Average cosine of angle between ray and normal
  float russian_roulette_survival;
  std::vector<RayEvent> events;
};

class RayAnalyzer {
 public:
  // Analyze a single ray through the scene
  PathMetrics analyzePath(const Ray& ray, const Scene& scene, int max_depth = 50);

  // Analyze multiple rays for a pixel
  std::vector<PathMetrics> analyzePixel(int x, int y, const Scene& scene, const Camera& camera, int samples = 100);

  // Get statistics from a collection of paths
  std::unordered_map<std::string, float> getPathStatistics(const std::vector<PathMetrics>& paths);

  // For a given pixel, correlate path characteristics with sample "fitness"
  void correlatePathsWithFitness(int x, int y, const Scene& scene, const Camera& camera, int samples = 100);

 private:
  // Helper to compute sample fitness (how good a sample is)
  float computeSampleFitness(const PathMetrics& path);

  // Helper to track a ray path with detailed instrumentation
  PathMetrics tracePath(const Ray& ray, const Scene& scene, int max_depth);
};

// }  // namespace debug
// }  // namespace glimpse

#endif