

#define MULTITHREADED
#ifdef MULTITHREADED
#include <future>
#include <thread>
#endif

#include "camera.h"
#include "common.h"
#include "hittables/bvh_node.h"
#include "hittables/quad.h"
#include "material.h"
#include "pdf.h"
#include "render.h"
#include "vec3.h"

#define USE_STRATIFIED 1

// Recursive ray tracing with depth limiting
color ray_color(const ray &r, const color &background, const hittable &world, int depth, const hittable &lights,
                bool has_lights) {
  hit_record rec;

  // if we've exceeded the ray bounce limit, no more light is gathered
  if (depth < 0) return color(0, 0, 0);

  // If the ray hits nothing, return the background color.
  // Use eps = 0.001 to avoid self-intersections
  if (world.hit(r, interval{0.001, infinity}, rec)) {
    scatter_record srec;
    color color_from_emission = rec.mat->emitted(r, rec, rec.u, rec.v, rec.p);

    // Scattered reflectance
    if (rec.mat->scatter(r, rec, srec)) {
      if (srec.skip_pdf) {
        return srec.attenuation * ray_color(srec.skip_pdf_ray, background, world, depth - 1, lights, has_lights);
      }

      ray scattered;
      double pdf_value;
      if (has_lights) {
        auto light_ptr = make_shared<hittable_pdf>(lights, rec.p);
        mixture_pdf mixed_pdf(light_ptr, srec.pdf_ptr);
        scattered = ray(rec.p, mixed_pdf.generate(), r.time());
        pdf_value = mixed_pdf.value(scattered.direction());
      } else {
        scattered = ray(rec.p, srec.pdf_ptr->generate(), r.time());
        pdf_value = srec.pdf_ptr->value(scattered.direction());
      }

      double scattering_pdf = rec.mat->scattering_pdf(r, rec, scattered);

      color sample_color = ray_color(scattered, background, world, depth - 1, lights, has_lights);

      color color_from_scatter = (srec.attenuation * scattering_pdf * sample_color) / pdf_value;

      return color_from_emission + color_from_scatter;
    }

    return color_from_emission;
  }

  return background;
  // vec3 unit_direction = unit_vector(r.direction());
  // auto t = 0.5 * (unit_direction.y() + 1.0);
  // return (1.0 - t) * color(1.0, 1.0, 1.0) + t * background;
}

vec3 sample_square_stratified(int s_i, int s_j, double recip_sqrt_spp) {
  // Returns the vector to a random point in the square sub-pixel specified by grid
  // indices s_i and s_j, for an idealized unit square pixel [-.5,-.5] to [+.5,+.5].

  auto px = ((s_i + random_double()) * recip_sqrt_spp) - 0.5;
  auto py = ((s_j + random_double()) * recip_sqrt_spp) - 0.5;

  return vec3(px, py, 0);
}

void render_section(Image &image, int start_row, int end_row, const Scene &scene, const bvh_node &world_bvh,
                    std::atomic<int> *progress = nullptr) {
  auto &cam = scene.cam;
  for (int j = end_row - 1; j >= start_row; --j) {
    for (int i = 0; i < cam.image_width; ++i) {
      color pixel_color(0, 0, 0);
#if USE_STRATIFIED == 0
      for (int s = 0; s < cam.samples_per_pixel; ++s) {
        // sample pixels with random offset for anti-aliasing
        auto u = (i + random_double(-0.5, 0.5)) / (cam.image_width - 1);
        auto v = (j + random_double(-0.5, 0.5)) / (cam.image_height - 1);
        ray r = cam.get_ray(u, v);
        pixel_color += ray_color(r, background, world_bvh, cam.max_depth);
        if (progress) (*progress)++;
      }
#elif USE_STRATIFIED == 1
      for (int s_j = 0; s_j < cam.sqrt_spp; ++s_j) {
        for (int s_i = 0; s_i < cam.sqrt_spp; ++s_i) {
          auto offset = sample_square_stratified(s_i, s_j, cam.recip_sqrt_spp);
          auto u = (i + offset.x()) / (cam.image_width - 1);
          auto v = (j + offset.y()) / (cam.image_height - 1);
          ray r = cam.get_ray(u, v);
          pixel_color +=
              ray_color(r, scene.background, world_bvh, cam.max_depth, scene.lights, !scene.lights.objects.empty());
          if (progress) (*progress)++;
        }
      }
#endif

      pixel_color = (pixel_color * cam.pixel_samples_scale);
      image.set(i, j, pixel_color);
    }
  }
}

void Renderer::render_scene(const Scene &scene, Image &image, std::atomic<int> *progress) {
  auto background = scene.background;

  // camera cam = scene.cam;
  // cam.initialize();
  // scene.cam.initialize();
  auto world_bvh = bvh_node(scene.world);

  // Image

#ifdef MULTITHREADED
  const int num_threads = std::thread::hardware_concurrency();
  std::vector<std::future<void>> futures;
  int rows_per_thread = scene.cam.image_height / num_threads;

  for (int t = 0; t < num_threads; ++t) {
    int start_row = t * rows_per_thread;
    int end_row = (t == num_threads - 1) ? scene.cam.image_height : start_row + rows_per_thread;

    futures.push_back(std::async(std::launch::async, render_section, std::ref(image), start_row, end_row,
                                 std::ref(scene), std::ref(world_bvh), progress));
  }

  for (auto &f : futures) {
    f.get();
  }
#else  // SINGLETHREADED

  // Image image(image_width, image_height);
  render_section(image, 0, image_height, image_width, image_height, samples_per_pixel, cam, background, world_bvh,
                 max_depth);

#endif
}