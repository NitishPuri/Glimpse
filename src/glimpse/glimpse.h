#pragma once
// clang-format off
/*
 ░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░▒▓██████████████▓▒░░▒▓███████▓▒░ ░▒▓███████▓▒░▒▓████████▓▒░ 
░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        
░▒▓█▓▒░      ░▒▓█▓▒░      ░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        
░▒▓█▓▒▒▓███▓▒░▒▓█▓▒░      ░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓███████▓▒░ ░▒▓██████▓▒░░▒▓██████▓▒░   
░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░             ░▒▓█▓▒░▒▓█▓▒░        
░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░             ░▒▓█▓▒░▒▓█▓▒░        
 ░▒▓██████▓▒░░▒▓████████▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓███████▓▒░░▒▓████████▓▒░
 */

#include "glimpse/util/vec3.h"
#include "glimpse/util/interval.h"
#include "glimpse/ray.h"
#include "glimpse/util/aabb.h"

#include "glimpse/hittables/hittable.h"

#include "glimpse/texture.h"
#include "glimpse/material.h"
#include "glimpse/perlin.h"

#include "glimpse/camera.h"
#include "glimpse/util/image.h"

// hittables
#include "glimpse/hittables/hittable_list.h"
#include "glimpse/hittables/quad.h"
#include "glimpse/hittables/sphere.h"
#include "glimpse/hittables/moving_sphere.h"

#include "glimpse/hittables/constant_medium.h"
#include "glimpse/hittables/bvh_node.h"

#include "glimpse/scenes.h"
// clang-format on
