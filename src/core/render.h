#pragma once

#include "image.h"
#include "scenes.h"

class Renderer {
 public:
  // static color ray_color(const ray &r, const color &background,
  //                        const hittable &world, int depth);

  // static void render_section(Image &image, int start_row, int end_row,
  //                            int image_width, int image_height,
  //                            int samples_per_pixel, const camera &cam,
  //                            const color &background, const bvh_node
  //                            &world_bvh, int max_depth, std::atomic<int>
  //                            *progress = nullptr);

  static void render_scene(const Scene &scene, Image &image,
                           std::atomic<int> *progress = nullptr);
};