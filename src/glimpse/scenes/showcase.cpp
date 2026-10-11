// Glimpse's own test and showcase scenes.

#include "glimpse/glimpse.h"
#include "glimpse/scenes/factories.h"

namespace glimpse {

Scene directionsTest() {
  HittableList objects;

  auto mat = make_shared<Lambertian>(Color(0.8, 0.3, 0.3));
  objects.add(make_shared<Sphere>(Point3(0, 0, 0), 0.5, mat));

  objects.add(make_shared<Sphere>(Point3(10, 0, 2), 0.5, make_shared<Lambertian>(Color(0.2, 0.8, 0.2))));
  objects.add(make_shared<Sphere>(Point3(-10, 0, -2), 0.5, make_shared<Lambertian>(Color(0.2, 0.2, 0.8))));

  objects.add(make_shared<Sphere>(Point3(0, 10, 0), 0.5, make_shared<Lambertian>(Color(0.2, 0.8, 0.8))));
  objects.add(make_shared<Sphere>(Point3(0, -10, 0), 0.5, make_shared<Lambertian>(Color(0.2, 0.2, 0.8))));

  Scene scene;
  scene.world = objects;
  scene.background = Color(0.7, 0.8, 1.0);
  scene.cam.lookfrom = Point3(0, 0, 10);
  scene.cam.lookat = Point3(0, 0, 0);
  scene.cam.vfov = 120.0;
  scene.cam.samplesPerPixel = 10;

  return scene;
}

Scene twoDiffuseSpheres() {
  HittableList objects;

  objects.add(make_shared<Sphere>(Point3(0, -1000, 0), 1000, make_shared<Lambertian>(Color(0.8, 0.8, 0.0))));
  objects.add(make_shared<Sphere>(Point3(0, 2, 0), 2, make_shared<Lambertian>(Color(0.8, 0.0, 0.8))));

  Scene scene;
  scene.world = objects;
  scene.background = Color(0.7, 0.8, 1.0);
  scene.cam.lookfrom = Point3(13, 2, 3);
  scene.cam.lookat = Point3(0, 0, 0);
  scene.cam.vfov = 40.0;

  return scene;
}

Scene materialShowcase() {
  HittableList objects;

  // diffuse
  objects.add(make_shared<Sphere>(Point3(-4, 2, -4), 2, make_shared<Lambertian>(Color(0.5, 0.8, 0.7))));
  objects.add(make_shared<Sphere>(Point3(2, 2, -6), 2, make_shared<Lambertian>(make_shared<NoiseTexture>(4))));

  // metal
  objects.add(make_shared<Sphere>(Point3(5, 2, -3), 2, make_shared<Metal>(Color(0.6, 0.2, 0.8), 0.2)));
  objects.add(make_shared<Sphere>(Point3(-8, 2, -4), 2, make_shared<Metal>(Color(0.3, 0.5, 0.8), 0.01)));

  // // "air bubble"
  objects.add(make_shared<Sphere>(Point3(0, 2, 0), 2, make_shared<Dielectric>(1.5)));
  objects.add(make_shared<Sphere>(Point3(0, 2, 0), 1.95, make_shared<Dielectric>(1.0 / 1.5)));

  // glass sphere
  objects.add(make_shared<Sphere>(Point3(-4, 2, 0), 2, make_shared<Dielectric>(1.33)));
  objects.add(make_shared<Sphere>(Point3(-4, 2, 0), 1.4, make_shared<Dielectric>(1.0 / 1.33)));

  // bouncing balls
  objects.add(make_shared<Sphere>(Point3(-8, 2, 2), Point3(-8, 2.2, 2), 2,
                                   make_shared<Metal>(Color(0.6, 0.2, 0.8), 0.04)));

  // ground
  objects.add(make_shared<Sphere>(Point3(0, -1000, 0), 1000, make_shared<Lambertian>(Color(0.8, 0.0, 0.8))));

  Scene scene;
  scene.world = objects;
  scene.background = Color(0.7, 0.8, 1.0);
  scene.cam.lookfrom = Point3(0, 7, 15);
  scene.cam.lookat = Point3(0, 0, 0);
  scene.cam.vfov = 60.0;

  return scene;
}

Scene untitled1() {
  HittableList objects;

  // diffuse
  objects.add(make_shared<Sphere>(Point3(-4, 2, -4), 2, make_shared<Lambertian>(Color(0.5, 0.8, 0.7))));
  objects.add(make_shared<Sphere>(Point3(2, 2, -6), 2, make_shared<Lambertian>(make_shared<NoiseTexture>(4))));

  // Giant Mirrors!
  shared_ptr<Material> aluminium = make_shared<Metal>(Color(0.8, 0.85, 0.88), 0.0);
  shared_ptr<NoiseTexture> nt = make_shared<NoiseTexture>(4);
  auto giantMirror = make_shared<Quad>(Point3{-20, 1, -14}, Vec3{200, 0, -100}, Vec3{0, 20, 0}, aluminium);
  objects.add(giantMirror);

  for (int i = 0; i < 13; ++i) {
    auto light1 = make_shared<Sphere>(Point3(5 * (i + 1), 2 + 2 * std::sin(i * 0.01), -4 + +10 * std::sin(i * 0.01)),
                                       2, make_shared<DiffuseLight>(nt));
    objects.add(light1);
  }

  // ground
  auto pertext = make_shared<NoiseTexture>(10, Color(.8, .4, .2));
  const auto groundSize = 10000;
  objects.add(make_shared<Sphere>(Point3(0, -groundSize, 0), groundSize, make_shared<Lambertian>(pertext)));

  Scene scene;
  scene.world = objects;
  scene.background = Color(0.3, 0.03, 0.4);
  scene.cam.samplesPerPixel = 5;
  scene.cam.lookfrom = Point3(0, 7, 25);
  scene.cam.lookat = Point3(0, 0, 0);
  scene.cam.vfov = 40.0;

  return scene;
}

}  // namespace glimpse
