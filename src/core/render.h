#pragma once

#define MULTITHREADED

#ifdef MULTITHREADED
#include <future>
#include <thread>
#endif

#include "glimpse.h"

color ray_color(const ray &r, const color &background, const hittable &world,
                int depth) {
  hit_record rec;

  // if we've exceeded the ray bounce limit, no more light is gathered
  if (depth < 0) return color(0, 0, 0);

  // If the ray hits nothing, return the background color.
  if (!world.hit(r, {0.001, infinity}, rec)) {
    return background;
  }

  ray scattered;
  color attenuation;
  color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);

  if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered)) return emitted;

  return emitted +
         attenuation * ray_color(scattered, background, world, depth - 1);

  // vec3 unit_direction = unit_vector(r.direction());
  // auto t = 0.5 * (unit_direction.y() + 1.0);
  // return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

void render_section(Image &image, int start_row, int end_row, int image_width,
                    int image_height, int samples_per_pixel, const camera &cam,
                    const color &background, const bvh_node &world_bvh,
                    int max_depth, std::atomic<int> *progress = nullptr) {
  double pixel_sample_scale = 1.0 / samples_per_pixel;
  for (int j = end_row - 1; j >= start_row; --j) {
    for (int i = 0; i < image_width; ++i) {
      color pixel_color(0, 0, 0);
      for (int s = 0; s < samples_per_pixel; ++s) {
        auto u = (i + random_double()) / (image_width - 1);
        auto v = (j + random_double()) / (image_height - 1);
        ray r = cam.get_ray(u, v);
        pixel_color += ray_color(r, background, world_bvh, max_depth);
        if (progress) (*progress)++;
      }
      pixel_color = sqrt(pixel_color * pixel_sample_scale);
      image.set(i, j, pixel_color);
    }
  }
}

void render_scene(const Scene &scene, Image &image,
                  std::atomic<int> *progress = nullptr) {
  auto aspect_ratio = scene.aspect_ratio;
  auto image_width = scene.image_width;
  auto image_height = static_cast<int>(image_width / aspect_ratio);
  auto samples_per_pixel = scene.samples_per_pixel;
  auto background = scene.background;
  auto max_depth = scene.max_depth;

  vec3 vup(0, 1, 0);
  double focal_length = 10.0;
  double time0 = 0.0;
  double time1 = 1.0;
  camera cam(scene.lookfrom, scene.lookat, vup, scene.vfov, scene.aspect_ratio,
             scene.aperture, focal_length, time0, time1);

  auto world_bvh = bvh_node(scene.world, 0, 1);

  // Image

#ifdef MULTITHREADED
  const int num_threads = std::thread::hardware_concurrency();
  std::vector<std::future<void>> futures;
  int rows_per_thread = image_height / num_threads;

  for (int t = 0; t < num_threads; ++t) {
    int start_row = t * rows_per_thread;
    int end_row =
        (t == num_threads - 1) ? image_height : start_row + rows_per_thread;

    // std::async(std::launch::async, render_section, std::ref(image),
    // start_row, end_row, image_width, image_height, samples_per_pixel,
    // std::ref(cam), std::ref(background), std::ref(world_bvh), max_depth);
    futures.push_back(std::async(
        std::launch::async, render_section, std::ref(image), start_row, end_row,
        image_width, image_height, samples_per_pixel, std::ref(cam),
        std::ref(background), std::ref(world_bvh), max_depth, progress));
  }

  for (auto &f : futures) {
    f.get();
  }
#else  // SINGLETHREADED

  // Image image(image_width, image_height);
  render_section(image, 0, image_height, image_width, image_height,
                 samples_per_pixel, cam, background, world_bvh, max_depth);

#endif
}