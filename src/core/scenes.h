#pragma once

#include <functional>
#include <unordered_map>

#include "camera.h"
#include "hittables/hittable_list.h"

namespace glimpse {

struct Scene {
  // world
  hittable_list world;

  hittable_list lights;

  // env
  color background = color(0.7, 0.8, 1.0);

  camera cam;

  static std::unordered_map<std::string, std::function<Scene()>> SceneMap;
  static std::vector<std::string> SceneNames;
};

}  // namespace glimpse