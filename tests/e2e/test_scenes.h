#pragma once

#include "glimpse/camera.h"
#include "glimpse/interaction.h"
#include "glimpse/shapes.h"
#include "glimpse/materials.h"
#include "glimpse/scene.h"
#include "glimpse/util/vecmath.h"

// Simple scene with a single sphere
inline Scene createSimpleSphereScene() {
  Scene scene;

  // Setup camera
  scene.cam.lookfrom = Point3(0, 0, 2);
  scene.cam.lookat = Point3(0, 0, 0);
  scene.cam.vup = Vec3(0, 1, 0);
  scene.cam.vfov = 60.0;
  scene.cam.aspectRatio = 16.0 / 9.0;
  scene.cam.defocusAngle = 0;  // No DoF

  // Grey background
  scene.background = Color(0.7, 0.8, 0.9);

  // Add a sphere
  auto mat = make_shared<Lambertian>(Color(0.5, 0.2, 0.2));
  scene.world.add(make_shared<Sphere>(Point3(0, 0, 0), 0.5, mat));

  return scene;
}

// Cornell box-like scene
inline Scene createCornellBoxScene() {
  Scene scene;

  // Setup camera for Cornell box
  scene.cam.lookfrom = Point3(278, 278, -800);
  scene.cam.lookat = Point3(278, 278, 0);
  scene.cam.vup = Vec3(0, 1, 0);
  scene.cam.vfov = 40.0;
  scene.cam.aspectRatio = 1.0;  // Square
  scene.cam.defocusAngle = 0;

  // Black background
  scene.background = Color(0, 0, 0);

  // Materials
  auto red = make_shared<Lambertian>(Color(.65, .05, .05));
  auto white = make_shared<Lambertian>(Color(.73, .73, .73));
  auto green = make_shared<Lambertian>(Color(.12, .45, .15));
  auto light = make_shared<DiffuseLight>(Color(15, 15, 15));

  // Box sides
  scene.world.add(make_shared<Quad>(Point3(555, 0, 0), Vec3(0, 0, 555), Vec3(0, 555, 0), green));
  scene.world.add(make_shared<Quad>(Point3(0, 0, 0), Vec3(0, 0, 555), Vec3(0, 555, 0), red));
  scene.world.add(make_shared<Quad>(Point3(0, 0, 0), Vec3(555, 0, 0), Vec3(0, 0, 555), white));
  scene.world.add(make_shared<Quad>(Point3(555, 555, 555), Vec3(-555, 0, 0), Vec3(0, 0, -555), white));
  scene.world.add(make_shared<Quad>(Point3(0, 555, 0), Vec3(555, 0, 0), Vec3(0, 0, 555), white));
  scene.world.add(make_shared<Quad>(Point3(0, 0, 555), Vec3(555, 0, 0), Vec3(0, 555, 0), white));

  // Light
  auto lightQuad = make_shared<Quad>(Point3(213, 554, 227), Vec3(130, 0, 0), Vec3(0, 0, 130), light);
  scene.world.add(lightQuad);
  scene.lights.add(lightQuad);

  // Two boxes
  auto box1 = make_shared<Sphere>(Point3(165, 90, 165), 80, white);
  auto box2 = make_shared<Sphere>(Point3(400, 150, 300), 110, white);
  scene.world.add(box1);
  scene.world.add(box2);

  return scene;
}

// Scene with reflective sphere
inline Scene createReflectiveSphereScene() {
  Scene scene;

  // Setup camera
  scene.cam.lookfrom = Point3(0, 1, 3);
  scene.cam.lookat = Point3(0, 0, 0);
  scene.cam.vup = Vec3(0, 1, 0);
  scene.cam.vfov = 50.0;
  scene.cam.aspectRatio = 16.0 / 9.0;
  scene.cam.defocusAngle = 0.6f;
  scene.cam.focusDistance = 3.0;

  // Sky background
  scene.background = Color(0.7, 0.8, 1.0);

  // Ground
  auto groundMat = make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
  scene.world.add(make_shared<Sphere>(Point3(0, -100.5, 0), 100, groundMat));

  // Center sphere - glass
  auto glassMat = make_shared<Dielectric>(1.5);
  scene.world.add(make_shared<Sphere>(Point3(0, 0, 0), 0.5, glassMat));

  // Left sphere - matte
  auto matteMat = make_shared<Lambertian>(Color(0.1, 0.2, 0.5));
  scene.world.add(make_shared<Sphere>(Point3(-1.0, 0, 0), 0.5, matteMat));

  // Right sphere - metal
  auto metalMat = make_shared<Metal>(Color(0.8, 0.6, 0.2), 0.0);
  scene.world.add(make_shared<Sphere>(Point3(1.0, 0, 0), 0.5, metalMat));

  return scene;
}

// Create a tiny scene with solid colors for simple testing
inline Scene createDebugScene() {
  Scene scene;

  // Setup simple camera
  scene.cam.lookfrom = Point3(0, 0, 2);
  scene.cam.lookat = Point3(0, 0, 0);
  scene.cam.vup = Vec3(0, 1, 0);
  scene.cam.vfov = 60.0;
  scene.cam.aspectRatio = 1.0;  // Square image

  // Set very small dimensions for quick testing
  scene.cam.imageWidth = 4;
  scene.cam.imageHeight = 4;

  // Set parameters for deterministic output
  scene.cam.samplesPerPixel = 1;  // Single sample per pixel
  scene.cam.maxDepth = 1;          // Single bounce

  // Solid blue background
  scene.background = Color(0.1, 0.0, 1.0);

  // Add a red sphere that takes up most of the image
  auto redMaterial = make_shared<Lambertian>(Color(1.0, 0, 0));
  scene.world.add(make_shared<Sphere>(Point3(0, 0, 0), 0.9, redMaterial));

  scene.cam.initialize();

  return scene;
}