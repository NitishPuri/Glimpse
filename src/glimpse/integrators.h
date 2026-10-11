#pragma once

// Integrators: the light transport estimators. Today one: rayColor, the RTIOW-style recursive path tracer.
// Techniques 07–09 add the iterative loop, next-event estimation and MIS alongside it (pbrt-v4 ch13).

#include "glimpse/interaction.h"
#include "glimpse/ray.h"

namespace glimpse {

// The RTIOW-style recursive path tracer. Lights are sampled through a mixture pdf when `hasLights`.
Color rayColor(const Ray &r, const Color &background, const Hittable &world,  //
                int depth, const Hittable &lights, bool hasLights);

}  // namespace glimpse
