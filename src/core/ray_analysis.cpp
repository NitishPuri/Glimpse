#include <algorithm>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <sstream>

#include "glimpse/debug/ray_analyzer.h"

namespace glimpse {
namespace debug {

PathMetrics RayAnalyzer::analyzePath(const Ray& ray, const Scene& scene, int max_depth) {
  return tracePath(ray, scene, max_depth);
}

PathMetrics RayAnalyzer::tracePath(const Ray& ray, const Scene& scene, int max_depth) {
  PathMetrics metrics;
  metrics.total_contribution = 0.0f;
  metrics.bounces = 0;
  metrics.diffuse_bounces = 0;
  metrics.specular_bounces = 0;
  metrics.transmission_bounces = 0;
  metrics.path_length = 0.0f;
  metrics.avg_cos_theta = 0.0f;
  metrics.russian_roulette_survival = 1.0f;

  Ray current_ray = ray;
  glm::vec3 throughput(1.0f);
  glm::vec3 last_position = ray.origin();
  HitRecord record;

  // Add initial ray creation event
  RayEvent create_event;
  create_event.type = RayEvent::CREATED;
  create_event.depth = 0;
  create_event.position = ray.origin();
  create_event.direction = ray.direction();
  create_event.contribution = 1.0f;
  metrics.events.push_back(create_event);

  for (int depth = 0; depth < max_depth; ++depth) {
    // Check for intersection
    if (!scene.hit(current_ray, 0.001f, std::numeric_limits<float>::infinity(), record)) {
      // Ray missed all objects
      RayEvent miss_event;
      miss_event.type = RayEvent::MISSED;
      miss_event.depth = depth;
      miss_event.position = current_ray.origin() + current_ray.direction() * 1000.0f;
      miss_event.direction = current_ray.direction();
      miss_event.contribution = glm::length(throughput);
      metrics.events.push_back(miss_event);

      // Add environment contribution
      glm::vec3 unit_dir = glm::normalize(current_ray.direction());
      float t = 0.5f * (unit_dir.y + 1.0f);
      glm::vec3 sky_color = (1.0f - t) * glm::vec3(1.0f) + t * glm::vec3(0.5f, 0.7f, 1.0f);
      metrics.total_contribution = glm::length(throughput * sky_color);
      break;
    }

    // Add hit event
    RayEvent hit_event;
    hit_event.type = RayEvent::HIT;
    hit_event.depth = depth;
    hit_event.position = record.p;
    hit_event.direction = current_ray.direction();
    hit_event.normal = record.normal;
    hit_event.material_name = record.mat_ptr->name();
    hit_event.contribution = glm::length(throughput);
    metrics.events.push_back(hit_event);

    // Update path metrics
    metrics.bounces++;
    metrics.path_length += glm::length(record.p - last_position);
    last_position = record.p;

    // Compute cos theta for this bounce
    float cos_theta = glm::dot(-glm::normalize(current_ray.direction()), record.normal);
    metrics.avg_cos_theta = (metrics.avg_cos_theta * (metrics.bounces - 1) + cos_theta) / metrics.bounces;

    // Scatter
    Ray scattered;
    glm::vec3 attenuation;

    if (record.mat_ptr->scatter(current_ray, record, attenuation, scattered)) {
      // Add scatter event
      RayEvent scatter_event;
      scatter_event.type = RayEvent::SCATTERED;
      scatter_event.depth = depth;
      scatter_event.position = record.p;
      scatter_event.direction = scattered.direction();
      scatter_event.attenuation = attenuation;
      scatter_event.material_name = record.mat_ptr->name();
      scatter_event.contribution = glm::length(throughput);
      scatter_event.pdf = 1.0f;  // This would come from your material's PDF if using importance sampling
      metrics.events.push_back(scatter_event);

      // Update throughput
      throughput *= attenuation;

      // Track bounce types
      if (record.mat_ptr->isSpecular()) {
        metrics.specular_bounces++;
      } else if (record.mat_ptr->isRefractive()) {
        metrics.transmission_bounces++;
      } else {
        metrics.diffuse_bounces++;
      }

      // Continue with scattered ray
      current_ray = scattered;

      // Russian roulette termination
      if (depth > 3) {
        float p = std::max(throughput.x, std::max(throughput.y, throughput.z));
        p = std::min(p, 0.95f);  // Cap to avoid infinite paths

        if (random_float() > p) {
          // Path terminated by Russian roulette
          metrics.russian_roulette_survival *= p;

          RayEvent absorb_event;
          absorb_event.type = RayEvent::ABSORBED;
          absorb_event.depth = depth;
          absorb_event.position = record.p;
          absorb_event.contribution = 0.0f;
          metrics.events.push_back(absorb_event);
          break;
        }

        metrics.russian_roulette_survival *= (1.0f / p);
        throughput /= p;
      }
    } else {
      // Material absorbed the ray
      RayEvent absorb_event;
      absorb_event.type = RayEvent::ABSORBED;
      absorb_event.depth = depth;
      absorb_event.position = record.p;
      absorb_event.material_name = record.mat_ptr->name();
      absorb_event.contribution = 0.0f;
      metrics.events.push_back(absorb_event);

      // Material emits light (simplified emission model)
      if (record.mat_ptr->isEmissive()) {
        glm::vec3 emitted = record.mat_ptr->emitted(record.u, record.v, record.p);
        metrics.total_contribution = glm::length(throughput * emitted);
      }
      break;
    }
  }

  return metrics;
}

std::vector<PathMetrics> RayAnalyzer::analyzePixel(int x, int y, const Scene& scene, const Camera& camera,
                                                   int samples) {
  std::vector<PathMetrics> results;
  results.reserve(samples);

  for (int s = 0; s < samples; ++s) {
    float u = (x + random_float()) / camera.image_width();
    float v = (y + random_float()) / camera.image_height();
    Ray ray = camera.get_ray(u, v);

    results.push_back(tracePath(ray, scene, 50));
  }

  return results;
}

std::unordered_map<std::string, float> RayAnalyzer::getPathStatistics(const std::vector<PathMetrics>& paths) {
  std::unordered_map<std::string, float> stats;

  if (paths.empty()) return stats;

  // Calculate basic statistics
  float avg_contrib = 0.0f;
  float avg_bounces = 0.0f;
  float avg_diffuse = 0.0f;
  float avg_specular = 0.0f;
  float avg_transmission = 0.0f;
  float avg_path_length = 0.0f;
  float avg_cos_theta = 0.0f;

  std::vector<float> contributions;
  contributions.reserve(paths.size());

  for (const auto& path : paths) {
    avg_contrib += path.total_contribution;
    avg_bounces += path.bounces;
    avg_diffuse += path.diffuse_bounces;
    avg_specular += path.specular_bounces;
    avg_transmission += path.transmission_bounces;
    avg_path_length += path.path_length;
    avg_cos_theta += path.avg_cos_theta;

    contributions.push_back(path.total_contribution);
  }

  int n = paths.size();
  avg_contrib /= n;
  avg_bounces /= n;
  avg_diffuse /= n;
  avg_specular /= n;
  avg_transmission /= n;
  avg_path_length /= n;
  avg_cos_theta /= n;

  // Calculate variance and standard deviation
  float variance = 0.0f;
  for (float contrib : contributions) {
    variance += (contrib - avg_contrib) * (contrib - avg_contrib);
  }
  variance /= n;
  float std_dev = std::sqrt(variance);

  // Store statistics
  stats["avg_contribution"] = avg_contrib;
  stats["avg_bounces"] = avg_bounces;
  stats["avg_diffuse_bounces"] = avg_diffuse;
  stats["avg_specular_bounces"] = avg_specular;
  stats["avg_transmission_bounces"] = avg_transmission;
  stats["avg_path_length"] = avg_path_length;
  stats["avg_cos_theta"] = avg_cos_theta;
  stats["variance"] = variance;
  stats["std_dev"] = std_dev;

  return stats;
}

float RayAnalyzer::computeSampleFitness(const PathMetrics& path) {
  // A simple fitness function - could be refined based on your needs
  // Higher is better
  float fitness = path.total_contribution;

  // Penalize high variance paths
  fitness /= (1.0f + path.variance);

  // Favor shorter paths slightly (efficiency)
  fitness *= std::exp(-0.05f * path.bounces);

  return fitness;
}

void RayAnalyzer::correlatePathsWithFitness(int x, int y, const Scene& scene, const Camera& camera, int samples) {
  auto paths = analyzePixel(x, y, scene, camera, samples);

  // Compute fitness for each path
  std::vector<float> fitness_scores;
  for (const auto& path : paths) {
    fitness_scores.push_back(computeSampleFitness(path));
  }

  // Calculate correlations between path characteristics and fitness

  // First compute means
  float mean_fitness = std::accumulate(fitness_scores.begin(), fitness_scores.end(), 0.0f) / samples;

  float mean_bounces = 0.0f;
  float mean_diffuse = 0.0f;
  float mean_specular = 0.0f;
  float mean_transmission = 0.0f;
  float mean_path_length = 0.0f;
  float mean_cos_theta = 0.0f;

  for (const auto& path : paths) {
    mean_bounces += path.bounces;
    mean_diffuse += path.diffuse_bounces;
    mean_specular += path.specular_bounces;
    mean_transmission += path.transmission_bounces;
    mean_path_length += path.path_length;
    mean_cos_theta += path.avg_cos_theta;
  }

  mean_bounces /= samples;
  mean_diffuse /= samples;
  mean_specular /= samples;
  mean_transmission /= samples;
  mean_path_length /= samples;
  mean_cos_theta /= samples;

  // Compute correlations
  float corr_bounces = 0.0f;
  float corr_diffuse = 0.0f;
  float corr_specular = 0.0f;
  float corr_transmission = 0.0f;
  float corr_path_length = 0.0f;
  float corr_cos_theta = 0.0f;

  for (int i = 0; i < samples; ++i) {
    const auto& path = paths[i];
    float fitness = fitness_scores[i];

    corr_bounces += (path.bounces - mean_bounces) * (fitness - mean_fitness);
    corr_diffuse += (path.diffuse_bounces - mean_diffuse) * (fitness - mean_fitness);
    corr_specular += (path.specular_bounces - mean_specular) * (fitness - mean_fitness);
    corr_transmission += (path.transmission_bounces - mean_transmission) * (fitness - mean_fitness);
    corr_path_length += (path.path_length - mean_path_length) * (fitness - mean_fitness);
    corr_cos_theta += (path.avg_cos_theta - mean_cos_theta) * (fitness - mean_fitness);
  }

  // Normalize to get correlation coefficients
  std::cout << "Path-Fitness Correlations for pixel (" << x << ", " << y << "):" << std::endl;
  std::cout << "Total bounces: " << corr_bounces << std::endl;
  std::cout << "Diffuse bounces: " << corr_diffuse << std::endl;
  std::cout << "Specular bounces: " << corr_specular << std::endl;
  std::cout << "Transmission bounces: " << corr_transmission << std::endl;
  std::cout << "Path length: " << corr_path_length << std::endl;
  std::cout << "Average cos(theta): " << corr_cos_theta << std::endl;
}

}  // namespace debug
}  // namespace glimpse