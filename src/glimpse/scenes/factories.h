#pragma once

// Scene factories, defined in the scenes/*.cpp family files and listed in registry.cpp.

#include "glimpse/scene.h"

namespace glimpse {

Scene randomScene();
Scene twoSpheresCheck();
Scene twoPerlinSpheres();
Scene earth();
Scene quads();
Scene simpleLight();
Scene finalScene();
Scene cornellBox();
Scene cornellSmoke();
Scene directionsTest();
Scene twoDiffuseSpheres();
Scene materialShowcase();
Scene untitled1();

}  // namespace glimpse
