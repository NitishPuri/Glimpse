#pragma once

#include "glimpse/interaction.h"
#include "glimpse/ray.h"

namespace glimpse {

// The RTIOW-style recursive path tracer. Lights are sampled through a mixture pdf when `has_lights`.
color ray_color(const ray &r, const color &background, const hittable &world,  //
                int depth, const hittable &lights, bool has_lights);

}  // namespace glimpse
