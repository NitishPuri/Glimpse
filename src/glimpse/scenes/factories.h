#pragma once

// Scene factories, defined in the scenes/*.cpp family files and listed in registry.cpp.

#include "glimpse/scene.h"

namespace glimpse {

Scene random_scene();
Scene two_spheres_check();
Scene two_perlin_spheres();
Scene earth();
Scene quads();
Scene simple_light();
Scene final_scene();
Scene cornell_box();
Scene cornell_smoke();
Scene directions_test();
Scene two_diffuse_spheres();
Scene material_showcase();
Scene untitled_1();

}  // namespace glimpse
