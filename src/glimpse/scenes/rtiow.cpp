// Scenes from the Ray Tracing in One Weekend books (book 1: randomScene; book 2: the rest).

#include "glimpse/glimpse.h"
#include "glimpse/scenes/factories.h"

namespace glimpse {

Scene randomScene() {
  HittableList world;

  auto checker = make_shared<CheckerTexture>(1, Color(0.2, 0.3, 0.1), Color(0.9, 0.9, 0.9));
  auto groundMaterial = make_shared<Lambertian>(checker);
  world.add(make_shared<Sphere>(Point3(0, -1000, 0), 1000, groundMaterial));

  for (int a = -11; a < 11; ++a) {
    for (int b = -11; b < 11; ++b) {
      auto chooseMat = randomDouble();
      Point3 center(a + 0.9 * randomDouble(), 0.2, b + 0.9 * randomDouble());

      if ((center - Point3(4, 0.2, 0)).length() > 0.9) {
        shared_ptr<Material> sphereMaterial;

        if (chooseMat < 0.9) {
          // diffuse
          auto albedo = Color::random() * Color::random();
          sphereMaterial = make_shared<Lambertian>(albedo);
          auto center2 = center + Vec3(0, randomDouble(0, 0.5), 0);
          world.add(make_shared<MovingSphere>(center, center2, 0.2, sphereMaterial));
        } else if (chooseMat < 0.95) {
          // metal
          auto albedo = Color::random(0.5, 1);
          auto fuzz = randomDouble(0, 0.5);
          sphereMaterial = make_shared<Metal>(albedo, fuzz);
          world.add(make_shared<Sphere>(center, 0.2, sphereMaterial));
        } else {
          // glass
          sphereMaterial = make_shared<Dielectric>(1.5);
          world.add(make_shared<Sphere>(center, 0.2, sphereMaterial));
        }
      }
    }
  }

  auto material1 = make_shared<Dielectric>(1.5);
  world.add(make_shared<Sphere>(Point3(0, 1, 0), 1.0, material1));

  auto material2 = make_shared<Lambertian>(Color(0.4, 0.2, 0.1));
  world.add(make_shared<Sphere>(Point3(-4, 1, 0), 1.0, material2));

  auto material3 = make_shared<Metal>(Color(0.7, 0.6, 0.5), 0.0);
  world.add(make_shared<Sphere>(Point3(4, 1, 0), 1.0, material3));

  Scene scene;
  scene.world = world;
  scene.background = Color(0.7, 0.8, 1.0);
  scene.cam.lookfrom = Point3(13, 2, 3);
  scene.cam.lookat = Point3(0, 0, 0);
  scene.cam.vfov = 20.0f;
  scene.cam.defocusAngle = 0.1f;

  return scene;
}

Scene twoSpheresCheck() {
  HittableList objects;

  auto checker = make_shared<CheckerTexture>(1, Color(0.2, 0.3, 0.1), Color(0.9, 0.9, 0.9));

  objects.add(make_shared<Sphere>(Point3(0, -10, 0), 10, make_shared<Lambertian>(checker)));
  objects.add(make_shared<Sphere>(Point3(0, 10, 0), 10, make_shared<Lambertian>(checker)));

  Scene scene;
  scene.world = objects;
  scene.background = Color(0.7, 0.8, 1.0);
  scene.cam.lookfrom = Point3(13, 2, 3);
  scene.cam.lookat = Point3(0, 0, 0);
  scene.cam.vfov = 20.0;

  return scene;
}

Scene twoPerlinSpheres() {
  HittableList objects;

  auto pertext = make_shared<NoiseTexture>(4);
  objects.add(make_shared<Sphere>(Point3(0, -1000, 0), 1000, make_shared<Lambertian>(pertext)));
  objects.add(make_shared<Sphere>(Point3(0, 2, 0), 2, make_shared<Lambertian>(pertext)));

  Scene scene;
  scene.world = objects;
  scene.background = Color(0.7, 0.8, 1.0);
  scene.cam.lookfrom = Point3(13, 2, 3);
  scene.cam.lookat = Point3(0, 0, 0);
  scene.cam.vfov = 20.0;

  return scene;
}

Scene earth() {
  auto earthTexture = make_shared<ImageTexture>("./res/earthmap.jpg");
  auto earthSurface = make_shared<Lambertian>(earthTexture);
  auto globe = make_shared<Sphere>(Point3(0, 0, 0), 2, earthSurface);

  Scene scene;
  scene.world = HittableList(globe);
  scene.background = Color(0.7, 0.8, 1.0);
  scene.cam.lookfrom = Point3(13, 2, 3);
  scene.cam.lookat = Point3(0, 0, 0);
  scene.cam.vfov = 20.0;

  return scene;
}

Scene quads() {
  HittableList world;

  // Materials
  auto leftRed = make_shared<Lambertian>(Color(1.0, 0.2, 0.2));
  auto backGreen = make_shared<Lambertian>(Color(0.2, 1.0, 0.2));
  auto rightBlue = make_shared<Lambertian>(Color(0.2, 0.2, 1.0));
  auto upperOrange = make_shared<Lambertian>(Color(1.0, 0.5, 0.0));
  auto lowerTeal = make_shared<Lambertian>(Color(0.2, 0.8, 0.8));

  // Quads
  world.add(make_shared<Quad>(Point3(-3, -2, 5), Vec3(0, 0, -4), Vec3(0, 4, 0), leftRed));
  world.add(make_shared<Quad>(Point3(-2, -2, 0), Vec3(4, 0, 0), Vec3(0, 4, 0), backGreen));
  world.add(make_shared<Quad>(Point3(3, -2, 1), Vec3(0, 0, 4), Vec3(0, 4, 0), rightBlue));
  world.add(make_shared<Quad>(Point3(-2, 3, 1), Vec3(4, 0, 0), Vec3(0, 0, 4), upperOrange));
  world.add(make_shared<Quad>(Point3(-2, -3, 5), Vec3(4, 0, 0), Vec3(0, 0, -4), lowerTeal));

  Scene scene;
  scene.world = world;
  scene.background = Color(0.7, 0.8, 1.0);
  scene.cam.aspectRatio = 1.0;
  scene.cam.imageWidth = 400;
  scene.cam.samplesPerPixel = 100;
  scene.cam.maxDepth = 50;

  scene.cam.vfov = 80;
  scene.cam.lookfrom = Point3(0, 0, 9);
  scene.cam.lookat = Point3(0, 0, 0);
  scene.cam.vup = Vec3(0, 1, 0);

  scene.cam.defocusAngle = 0;

  return scene;
}

Scene simpleLight() {
  HittableList objects;

  auto pertext = make_shared<NoiseTexture>(4);
  objects.add(make_shared<Sphere>(Point3(0, -1000, 0), 1000, make_shared<Lambertian>(pertext)));
  objects.add(make_shared<Sphere>(Point3(0, 2, 0), 2, make_shared<Lambertian>(pertext)));

  auto difflight = make_shared<DiffuseLight>(Color(4, 4, 4));
  auto sphereLight = make_shared<Sphere>(Point3(0, 7, 0), 2, difflight);
  auto quadLight = make_shared<Quad>(Point3{3, 1, -2}, Vec3{2, 0, 0}, Vec3{0, 2, 0}, difflight);
  objects.add(sphereLight);
  objects.add(quadLight);

  HittableList lightsList;
  lightsList.add(sphereLight);
  lightsList.add(quadLight);

  Scene scene;
  scene.world = objects;
  scene.lights = lightsList;
  scene.cam.samplesPerPixel = 400;
  scene.background = Color(0, 0, 0);
  scene.cam.lookfrom = Point3(26, 3, 6);
  scene.cam.lookat = Point3(0, 2, 0);
  scene.cam.vfov = 20.0;

  return scene;
}

Scene finalScene() {
  HittableList boxes1;
  auto ground = make_shared<Lambertian>(Color(0.48, 0.83, 0.53));

  const int boxesPerSide = 20;
  for (int i = 0; i < boxesPerSide; i++) {
    for (int j = 0; j < boxesPerSide; j++) {
      auto w = 100.0;
      auto x0 = -1000.0 + i * w;
      auto z0 = -1000.0 + j * w;
      auto y0 = 0.0;
      auto x1 = x0 + w;
      auto y1 = randomDouble(1, 101);
      auto z1 = z0 + w;

      boxes1.add(box(Point3(x0, y0, z0), Point3(x1, y1, z1), ground));
    }
  }

  // std::cout << "Final scene,.. \n" << std::endl;

  HittableList world;

  world.add(make_shared<BVHNode>(boxes1));

  auto light = make_shared<DiffuseLight>(Color(7, 7, 7));
  world.add(make_shared<Quad>(Point3(123, 554, 147), Vec3(300, 0, 0), Vec3(0, 0, 265), light));
  // objects.add(make_shared<xzRect>(123, 423, 147, 412, 554, light));

  auto center1 = Point3(400, 400, 200);
  auto center2 = center1 + Vec3(30, 0, 0);
  auto movingSphereMaterial = make_shared<Lambertian>(Color(0.7, 0.3, 0.1));
  world.add(make_shared<MovingSphere>(center1, center2, 50, movingSphereMaterial));

  world.add(make_shared<Sphere>(Point3(260, 150, 45), 50, make_shared<Dielectric>(1.5)));
  world.add(make_shared<Sphere>(Point3(0, 150, 145), 50, make_shared<Metal>(Color(0.8, 0.8, 0.9), 1.0)));

  auto boundary = make_shared<Sphere>(Point3(360, 150, 145), 70, make_shared<Dielectric>(1.5));
  world.add(boundary);
  world.add(make_shared<ConstantMedium>(boundary, 0.2, Color(0.2, 0.4, 0.9)));
  boundary = make_shared<Sphere>(Point3(0, 0, 0), 5000, make_shared<Dielectric>(1.5));
  world.add(make_shared<ConstantMedium>(boundary, .0001, Color(1, 1, 1)));

  auto emat = make_shared<Lambertian>(make_shared<ImageTexture>("./res/earthmap.jpg"));
  world.add(make_shared<Sphere>(Point3(400, 200, 400), 100, emat));
  auto pertext = make_shared<NoiseTexture>(0.1);
  world.add(make_shared<Sphere>(Point3(220, 280, 300), 80, make_shared<Lambertian>(pertext)));

  HittableList boxes2;
  auto white = make_shared<Lambertian>(Color(.73, .73, .73));
  int ns = 1000;
  for (int j = 0; j < ns; j++) {
    boxes2.add(make_shared<Sphere>(Point3::random(0, 165), 10, white));
  }

  world.add(make_shared<Translate>(make_shared<RotateY>(make_shared<BVHNode>(boxes2), 15), Vec3(-100, 270, 395)));

  // lights
  auto emptyMaterial = shared_ptr<Material>();
  HittableList lightsList;
  lightsList.add(make_shared<Quad>(Point3(123, 554, 147), Vec3(300, 0, 0), Vec3(0, 0, 265), emptyMaterial));

  Scene scene;
  scene.world = world;
  scene.cam.aspectRatio = 1.0;
  scene.cam.imageWidth = 800;
  scene.cam.samplesPerPixel = 1000;
  scene.background = Color(0, 0, 0);
  scene.cam.lookfrom = Point3(478, 278, -600);
  scene.cam.lookat = Point3(278, 278, 0);
  scene.cam.vfov = 40.0;

  return scene;
}

}  // namespace glimpse
