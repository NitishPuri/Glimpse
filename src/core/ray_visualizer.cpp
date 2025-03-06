#if 0

#include "ray_visualizer.h"

// #include <GL/glew.h>
// #include <GLFW/glfw3.h>

#include "hittables/hittable.h"
#include "render.h"

// namespace glimpse {
// namespace debug {

void RayVisualizer::trackRay(const ray& ray, const Scene& scene, int max_depth) {
  clear();
  tracePath(ray, scene, max_depth);
}

void RayVisualizer::tracePath(const ray& ray_, const Scene& scene, int max_depth) {
  ray current_ray = ray_;
  vec3 current_point = ray_.origin();
  vec3 throughput(1.0, 1.0, 1.0);
  hit_record record;

  for (int depth = 0; depth < max_depth; ++depth) {
    // Check for intersection
    if (!scene.hit(current_ray, 0.001f, std::numeric_limits<float>::infinity(), record)) {
      // Ray hit nothing, add segment to sky
      glm::vec3 end_point = current_ray.origin() + current_ray.direction() * 1000.0f;
      m_path_segments.push_back({current_point, end_point, glm::vec3(0.7f, 0.7f, 1.0f),  // sky color
                                 depth, "sky", glm::length(throughput)});
      break;
    }

    // Add path segment from last point to hit point
    glm::vec3 hit_point = record.p;
    m_path_segments.push_back({current_point, hit_point, throughput, depth, "travel", glm::length(throughput)});

    // Update current point
    current_point = hit_point;

    // Scatter (simplified from your renderer)
    Ray scattered;
    glm::vec3 attenuation;
    std::string event_type;

    if (record.mat_ptr->scatter(current_ray, record, attenuation, scattered)) {
      // Determine event type based on material properties
      if (record.mat_ptr->isSpecular()) {
        event_type = "reflection";
      } else if (record.mat_ptr->isRefractive()) {
        event_type = "refraction";
      } else {
        event_type = "diffuse";
      }

      // Update throughput
      throughput *= attenuation;

      // Continue with scattered ray
      current_ray = scattered;

      // Store this event
      m_path_segments.push_back(
          {hit_point,
           hit_point + record.normal * 0.1f,                                            // Small normal vector
           glm::vec3(record.normal.x, record.normal.y, record.normal.z) * 0.5f + 0.5f,  // Visualize normal
           depth, event_type, glm::length(throughput)});
    } else {
      // Absorption event
      m_path_segments.push_back({hit_point, hit_point, glm::vec3(1.0f, 0.0f, 0.0f),  // Red for absorption
                                 depth, "absorption", 0.0f});
      break;
    }

    // Russian roulette termination
    if (depth > 3) {
      float p = std::max(throughput.x, std::max(throughput.y, throughput.z));
      if (random_float() > p) {
        break;
      }
      throughput /= p;
    }
  }
}

void RayVisualizer::trackRegion(int x0, int y0, int x1, int y1, const Scene& scene, const Camera& camera) {
  clear();

  // Sample a few rays from the selected region
  const int samples = 5;  // Limited number for visualization

  for (int y = y0; y <= y1; ++y) {
    for (int x = x0; x <= x1; ++x) {
      for (int s = 0; s < samples; ++s) {
        float u = (x + random_float()) / camera.image_width();
        float v = (y + random_float()) / camera.image_height();
        Ray ray = camera.get_ray(u, v);
        tracePath(ray, scene, 10);
      }
    }
  }
}

void RayVisualizer::drawPaths(GLFWwindow* window, const Camera& view_camera) {
  // Setup OpenGL for line drawing
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  // Use view camera's projection parameters
  float aspect = view_camera.aspect_ratio();
  float vfov = view_camera.vertical_fov();
  glm::mat4 projection = glm::perspective(glm::radians(vfov), aspect, 0.1f, 100.0f);
  glm::mat4 view = glm::lookAt(view_camera.position(), view_camera.position() + view_camera.front(), view_camera.up());

  // Set view and projection matrices
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Draw path segments
  glLineWidth(2.0f);
  glBegin(GL_LINES);

  for (const auto& segment : m_path_segments) {
    if (segment.event_type == "travel") {
      // Use throughput for color, with alpha based on importance
      float alpha = std::min(1.0f, segment.importance);
      glColor4f(segment.color.r, segment.color.g, segment.color.b, alpha);
      glVertex3f(segment.start.x, segment.start.y, segment.start.z);
      glVertex3f(segment.end.x, segment.end.y, segment.end.z);
    }
  }

  glEnd();

  // Draw interaction points (reflections, refractions, etc.)
  glPointSize(5.0f);
  glBegin(GL_POINTS);

  for (const auto& segment : m_path_segments) {
    if (segment.event_type != "travel") {
      if (segment.event_type == "reflection") {
        glColor3f(1.0f, 0.8f, 0.0f);  // Gold for reflections
      } else if (segment.event_type == "refraction") {
        glColor3f(0.0f, 0.8f, 1.0f);  // Cyan for refractions
      } else if (segment.event_type == "diffuse") {
        glColor3f(0.0f, 1.0f, 0.0f);  // Green for diffuse
      } else if (segment.event_type == "absorption") {
        glColor3f(1.0f, 0.0f, 0.0f);  // Red for absorption
      } else if (segment.event_type == "sky") {
        glColor3f(0.7f, 0.7f, 1.0f);  // Light blue for sky
      }

      glVertex3f(segment.start.x, segment.start.y, segment.start.z);
    }
  }

  glEnd();
}

// }  // namespace debug
// }  // namespace glimpse
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////