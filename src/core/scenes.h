#pragma once

#include <functional>
#include <unordered_map>

#include "glimpse.h"

struct Scene {
  // world
  hittable_list world;

  // env
  color background = color(0.7, 0.8, 1.0);

  // camera
  point3 lookfrom;
  point3 lookat;
  float vfov;
  float aperture;

  double aspect_ratio = 16.0 / 9.0;
  int image_width = 800;

  // render
  int samples_per_pixel = 100;
  int max_depth = 50;

  static std::unordered_map<std::string, std::function<Scene()>> SceneMap;
  static std::vector<std::string> SceneNames;
};
