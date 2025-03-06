#pragma once

#include "core/camera.h"
#include "core/color.h"
#include "core/hittables/hittable.h"
#include "core/hittables/quad.h"
#include "core/hittables/sphere.h"
#include "core/material.h"
#include "core/scenes.h"
#include "core/vec3.h"
// #include "materials/diffuse_light.h"
// #include "materials/lambertian.h"
// #include "materials/metal.h"
// #include "textures/solid_color.h"

// Simple scene with a single sphere
Scene create_simple_sphere_scene() {
  Scene scene;

  // Setup camera
  scene.cam.lookfrom = point3(0, 0, 2);
  scene.cam.lookat = point3(0, 0, 0);
  scene.cam.vup = vec3(0, 1, 0);
  scene.cam.vfov = 60.0;
  scene.cam.aspect_ratio = 16.0 / 9.0;
  scene.cam.defocus_angle = 0;  // No DoF

  // Grey background
  scene.background = color(0.7, 0.8, 0.9);

  // Add a sphere
  auto material = make_shared<lambertian>(color(0.5, 0.2, 0.2));
  scene.world.add(make_shared<sphere>(point3(0, 0, 0), 0.5, material));

  return scene;
}

// Cornell box-like scene
Scene create_cornell_box_scene() {
  Scene scene;

  // Setup camera for Cornell box
  scene.cam.lookfrom = point3(278, 278, -800);
  scene.cam.lookat = point3(278, 278, 0);
  scene.cam.vup = vec3(0, 1, 0);
  scene.cam.vfov = 40.0;
  scene.cam.aspect_ratio = 1.0;  // Square
  scene.cam.defocus_angle = 0;

  // Black background
  scene.background = color(0, 0, 0);

  // Materials
  auto red = make_shared<lambertian>(color(.65, .05, .05));
  auto white = make_shared<lambertian>(color(.73, .73, .73));
  auto green = make_shared<lambertian>(color(.12, .45, .15));
  auto light = make_shared<diffuse_light>(color(15, 15, 15));

  // Box sides
  scene.world.add(make_shared<quad>(point3(555, 0, 0), vec3(0, 0, 555), vec3(0, 555, 0), green));
  scene.world.add(make_shared<quad>(point3(0, 0, 0), vec3(0, 0, 555), vec3(0, 555, 0), red));
  scene.world.add(make_shared<quad>(point3(0, 0, 0), vec3(555, 0, 0), vec3(0, 0, 555), white));
  scene.world.add(make_shared<quad>(point3(555, 555, 555), vec3(-555, 0, 0), vec3(0, 0, -555), white));
  scene.world.add(make_shared<quad>(point3(0, 555, 0), vec3(555, 0, 0), vec3(0, 0, 555), white));
  scene.world.add(make_shared<quad>(point3(0, 0, 555), vec3(555, 0, 0), vec3(0, 555, 0), white));

  // Light
  auto light_quad = make_shared<quad>(point3(213, 554, 227), vec3(130, 0, 0), vec3(0, 0, 130), light);
  scene.world.add(light_quad);
  scene.lights.add(light_quad);

  // Two boxes
  auto box1 = make_shared<sphere>(point3(165, 90, 165), 80, white);
  auto box2 = make_shared<sphere>(point3(400, 150, 300), 110, white);
  scene.world.add(box1);
  scene.world.add(box2);

  return scene;
}

// Scene with reflective sphere
Scene create_reflective_sphere_scene() {
  Scene scene;

  // Setup camera
  scene.cam.lookfrom = point3(0, 1, 3);
  scene.cam.lookat = point3(0, 0, 0);
  scene.cam.vup = vec3(0, 1, 0);
  scene.cam.vfov = 50.0;
  scene.cam.aspect_ratio = 16.0 / 9.0;
  scene.cam.defocus_angle = 0.6f;
  scene.cam.focus_distance = 3.0;

  // Sky background
  scene.background = color(0.7, 0.8, 1.0);

  // Ground
  auto ground_mat = make_shared<lambertian>(color(0.5, 0.5, 0.5));
  scene.world.add(make_shared<sphere>(point3(0, -100.5, 0), 100, ground_mat));

  // Center sphere - glass
  auto glass_mat = make_shared<dielectric>(1.5);
  scene.world.add(make_shared<sphere>(point3(0, 0, 0), 0.5, glass_mat));

  // Left sphere - matte
  auto matte_mat = make_shared<lambertian>(color(0.1, 0.2, 0.5));
  scene.world.add(make_shared<sphere>(point3(-1.0, 0, 0), 0.5, matte_mat));

  // Right sphere - metal
  auto metal_mat = make_shared<metal>(color(0.8, 0.6, 0.2), 0.0);
  scene.world.add(make_shared<sphere>(point3(1.0, 0, 0), 0.5, metal_mat));

  return scene;
}