

#define MULTITHREADED
#ifdef MULTITHREADED
#include <future>
#include <thread>
#endif

#include "glimpse/camera.h"
#include "glimpse/util/common.h"
#include "glimpse/aggregates.h"
#include "glimpse/shapes.h"
#include "glimpse/materials.h"
#include "glimpse/integrators.h"
#include "glimpse/render.h"
#include "glimpse/util/vec3.h"

namespace glimpse {

std::atomic<bool> Renderer::stop_rendering(false);

vec3 sample_square_stratified(int s_i, int s_j, double recip_sqrt_spp) {
  // Returns the vector to a random point in the square sub-pixel specified by grid
  // indices s_i and s_j, for an idealized unit square pixel [-.5,-.5] to [+.5,+.5].

  auto px = ((s_i + random_double()) * recip_sqrt_spp) - 0.5;
  auto py = ((s_j + random_double()) * recip_sqrt_spp) - 0.5;

  return vec3(px, py, 0);
}

struct RenderSectionArgs {
  Image &image;  // Image output, filled with rgb values
  Film &film;    // Intermediate buffer, accumulates samples and computes variance.
  int start_row;
  int end_row;
  const Scene &scene;
  const bvh_node &world_bvh;
  std::atomic<int> *progress;
};

void render_section(RenderSectionArgs &args) {
  auto &image = args.image;
  auto &cam = args.scene.cam;
  auto &start_row = args.start_row;
  auto &end_row = args.end_row;
  auto &world_bvh = args.world_bvh;
  auto &scene = args.scene;
  auto progress = args.progress;
  auto &film = args.film;

  for (int s_j = 0; s_j < cam.sqrt_spp; ++s_j) {
    for (int s_i = 0; s_i < cam.sqrt_spp; ++s_i) {
      for (int j = end_row - 1; j >= start_row; --j) {
        // Checked once per row: cheap, and a stop (viewer closing, Stop button) takes effect within a row.
        if (Renderer::stop_rendering.load()) return;
        for (int i = 0; i < cam.image_width; ++i) {
          // color pixel_color(0, 0, 0);
          color pixel_color(0, 0, 0);

          auto offset = sample_square_stratified(s_i, s_j, cam.recip_sqrt_spp);
          auto u = (i + offset.x()) / (cam.image_width - 1);
          auto v = (j + offset.y()) / (cam.image_height - 1);
          ray r = cam.get_ray(u, v);
          pixel_color +=
              ray_color(r, scene.background, world_bvh, cam.max_depth, scene.lights, !scene.lights.objects.empty());
          if (progress) (*progress)++;

          film.add_sample(i, j, pixel_color);
          pixel_color = film.get_sample(i, j);
          pixel_color = sqrt(pixel_color);  // gamma correction!
          // image.set(i, j, ImageColor{float(pixel_color.x()), float(pixel_color.y()), float(pixel_color.z())});
          // j counts up from the bottom (camera v); Image rows count down from the top, like image files.
          image.set_float(i, cam.image_height - 1 - j, static_cast<float>(pixel_color.x()),
                          static_cast<float>(pixel_color.y()), static_cast<float>(pixel_color.z()));
        }
      }
    }
  }
  std::cout << "Section done ... rows[" << start_row << ", " << end_row << "] thread " << std::this_thread::get_id()
            << std::endl;
}

void render_section_uncap(RenderSectionArgs &args) {
  auto &image = args.image;
  auto &cam = args.scene.cam;
  auto &start_row = args.start_row;
  auto &end_row = args.end_row;
  auto &world_bvh = args.world_bvh;
  auto &scene = args.scene;
  auto progress = args.progress;
  auto &film = args.film;

  while (!Renderer::stop_rendering.load()) {
    for (int s_j = 0; s_j < cam.sqrt_spp; ++s_j) {
      for (int s_i = 0; s_i < cam.sqrt_spp; ++s_i) {
        for (int j = end_row - 1; j >= start_row; --j) {
          for (int i = 0; i < cam.image_width; ++i) {
            // color pixel_color(0, 0, 0);
            color pixel_color(0, 0, 0);

            auto offset = sample_square_stratified(s_i, s_j, cam.recip_sqrt_spp);
            auto u = (i + offset.x()) / (cam.image_width - 1);
            auto v = (j + offset.y()) / (cam.image_height - 1);
            ray r = cam.get_ray(u, v);
            pixel_color +=
                ray_color(r, scene.background, world_bvh, cam.max_depth, scene.lights, !scene.lights.objects.empty());
            if (progress) (*progress)++;

            film.add_sample(i, j, pixel_color);
            pixel_color = film.get_sample(i, j);
            pixel_color = sqrt(pixel_color);  // gamma correction!
            // image.set(i, j, ImageColor{float(pixel_color.x()), float(pixel_color.y()), float(pixel_color.z())});
            // j counts up from the bottom (camera v); Image rows count down from the top, like image files.
            image.set_float(i, cam.image_height - 1 - j, static_cast<float>(pixel_color.x()),
                            static_cast<float>(pixel_color.y()), static_cast<float>(pixel_color.z()));
          }
        }
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  std::cout << "Stopping section ... rows[" << start_row << ", " << end_row << "] thread " << std::this_thread::get_id()
            << std::endl;
}

void Renderer::render_scene(Scene scene, Image &image, std::atomic<int> *progress) {
  // Derived camera fields (image_height, sqrt_spp, viewport) must be current. Callers set public fields and
  // may forget initialize(); it only recomputes, so calling it again is harmless.
  scene.cam.initialize();

  auto world_bvh = bvh_node(scene.world);

  film.initialize(scene.cam.image_width, scene.cam.image_height);

  // Fixed seed!
  // Random::set_seed(42);

#ifdef MULTITHREADED
  const int num_threads = std::thread::hardware_concurrency();
  std::vector<std::future<void>> futures;
  int rows_per_thread = scene.cam.image_height / num_threads;

  for (int t = 0; t < num_threads; ++t) {
    int start_row = t * rows_per_thread;
    int end_row = (t == num_threads - 1) ? scene.cam.image_height : start_row + rows_per_thread;

    RenderSectionArgs args{image, film, start_row, end_row, scene, world_bvh, progress};
    if (scene.cam.uncapped_spp) {
      futures.push_back(                  //
          std::async(std::launch::async,  //
                     [args]() mutable {   //
                       render_section_uncap(args);
                     }));
    } else {
      futures.push_back(                  //
          std::async(std::launch::async,  //
                     [args]() mutable {   //
                       render_section(args);
                     }));
    }
  }

  // Wait for a signal to stop rendering before waiting for threaads to finish.
  // while (!stop_rendering.load()) {
  //   std::this_thread::sleep_for(std::chrono::milliseconds(10));
  // }

  for (auto &f : futures) {
    f.get();
  }

  // TODO: Implement __FUNCTION__ logging with logger!?..
  // control logging from the logger class
  std::cout << __FUNCTION__ << " : All threads finished rendering..." << std::endl;
  Renderer::stop_rendering = false;
#else  // SINGLETHREADED

  // Image image(image_width, image_height);
  render_section(image, 0, image_height, image_width, image_height, samples_per_pixel, cam, background, world_bvh,
                 max_depth);

#endif
}

}  // namespace glimpse