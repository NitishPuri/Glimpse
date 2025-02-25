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

#include "vec3.h"
#include "color.h"
#include "interval.h"
#include "ray.h"
#include "aabb.h"

#include "hittables/hittable.h"

#include "texture.h"
#include "material.h"
#include "perlin.h"

#include "camera.h"
#include "image.h"

// hittables
#include "hittables/hittable_list.h"
#include "hittables/aarect.h"
#include "hittables/quad.h"
#include "hittables/box.h"
#include "hittables/sphere.h"
#include "hittables/moving_sphere.h"

#include "hittables/constant_medium.h"
#include "hittables/bvh_node.h"

#include "scenes.h"
// clang-format on
