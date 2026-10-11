// Glimpse's own test and showcase scenes.

#include "glimpse/glimpse.h"
#include "glimpse/scenes/factories.h"

namespace glimpse {

Scene directions_test() {
  hittable_list objects;

  auto material = make_shared<lambertian>(color(0.8, 0.3, 0.3));
  objects.add(make_shared<sphere>(point3(0, 0, 0), 0.5, material));

  objects.add(make_shared<sphere>(point3(10, 0, 2), 0.5, make_shared<lambertian>(color(0.2, 0.8, 0.2))));
  objects.add(make_shared<sphere>(point3(-10, 0, -2), 0.5, make_shared<lambertian>(color(0.2, 0.2, 0.8))));

  objects.add(make_shared<sphere>(point3(0, 10, 0), 0.5, make_shared<lambertian>(color(0.2, 0.8, 0.8))));
  objects.add(make_shared<sphere>(point3(0, -10, 0), 0.5, make_shared<lambertian>(color(0.2, 0.2, 0.8))));

  // objects.add(
  //     make_shared<sphere>(point3(0, -100.5, -1), 100,
  //                         make_shared<lambertian>(color(0.8, 0.8, 0.0))));

  Scene scene;
  scene.world = objects;
  scene.background = color(0.7, 0.8, 1.0);
  scene.cam.lookfrom = point3(0, 0, 10);
  scene.cam.lookat = point3(0, 0, 0);
  scene.cam.vfov = 120.0;
  scene.cam.samples_per_pixel = 10;

  return scene;
}

Scene two_diffuse_spheres() {
  hittable_list objects;

  objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(color(0.8, 0.8, 0.0))));
  objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(color(0.8, 0.0, 0.8))));

  Scene scene;
  scene.world = objects;
  scene.background = color(0.7, 0.8, 1.0);
  scene.cam.lookfrom = point3(13, 2, 3);
  scene.cam.lookat = point3(0, 0, 0);
  scene.cam.vfov = 40.0;

  return scene;
}

Scene material_showcase() {
  hittable_list objects;

  // diffuse
  objects.add(make_shared<sphere>(point3(-4, 2, -4), 2, make_shared<lambertian>(color(0.5, 0.8, 0.7))));
  objects.add(make_shared<sphere>(point3(2, 2, -6), 2, make_shared<lambertian>(make_shared<noise_texture>(4))));

  // metal
  objects.add(make_shared<sphere>(point3(5, 2, -3), 2, make_shared<metal>(color(0.6, 0.2, 0.8), 0.2)));
  objects.add(make_shared<sphere>(point3(-8, 2, -4), 2, make_shared<metal>(color(0.3, 0.5, 0.8), 0.01)));

  // // "air bubble"
  objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<dielectric>(1.5)));
  objects.add(make_shared<sphere>(point3(0, 2, 0), 1.95, make_shared<dielectric>(1.0 / 1.5)));

  // glass sphere
  objects.add(make_shared<sphere>(point3(-4, 2, 0), 2, make_shared<dielectric>(1.33)));
  objects.add(make_shared<sphere>(point3(-4, 2, 0), 1.4, make_shared<dielectric>(1.0 / 1.33)));

  // bouncing balls
  objects.add(make_shared<moving_sphere>(point3(-8, 2, 2), point3(-8, 2.2, 2), 2,
                                         make_shared<metal>(color(0.6, 0.2, 0.8), 0.04)));

  // TODO: add light

  // ground
  objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(color(0.8, 0.0, 0.8))));

  Scene scene;
  scene.world = objects;
  scene.background = color(0.7, 0.8, 1.0);
  scene.cam.lookfrom = point3(0, 7, 15);
  scene.cam.lookat = point3(0, 0, 0);
  scene.cam.vfov = 60.0;

  return scene;
}

Scene untitled_1() {
  hittable_list objects;
  hittable_list lights_list;

  // // diffuse
  objects.add(make_shared<sphere>(point3(-4, 2, -4), 2, make_shared<lambertian>(color(0.5, 0.8, 0.7))));
  objects.add(make_shared<sphere>(point3(2, 2, -6), 2, make_shared<lambertian>(make_shared<noise_texture>(4))));

  // // metal
  // objects.add(make_shared<sphere>(point3(5, 2, -3), 2, make_shared<metal>(color(0.6, 0.2, 0.8), 0.2)));
  // objects.add(make_shared<sphere>(point3(-8, 2, -4), 2, make_shared<metal>(color(0.3, 0.5, 0.8), 0.01)));

  // // // "air bubble"
  // objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<dielectric>(1.5)));
  // objects.add(make_shared<sphere>(point3(0, 2, 0), 1.95, make_shared<dielectric>(1.0 / 1.5)));

  // // glass sphere
  // objects.add(make_shared<sphere>(point3(-4, 2, 0), 2, make_shared<dielectric>(1.33)));
  // objects.add(make_shared<sphere>(point3(-4, 2, 0), 1.4, make_shared<dielectric>(1.0 / 1.33)));

  // // bouncing balls
  // objects.add(make_shared<moving_sphere>(point3(-8, 2, 2), point3(-8, 2.2, 2), 2,
  //                                        make_shared<metal>(color(0.6, 0.2, 0.8), 0.04)));

  // Giant Mirrors!
  shared_ptr<material> aluminium = make_shared<metal>(color(0.8, 0.85, 0.88), 0.0);
  shared_ptr<noise_texture> nt = make_shared<noise_texture>(4);
  auto giant_mirror = make_shared<quad>(point3{-20, 1, -14}, vec3{200, 0, -100}, vec3{0, 20, 0}, aluminium);
  objects.add(giant_mirror);

  for (int i = 0; i < 13; ++i) {
    color c = color::random();
    // noise_texture nt(4);
    // TODO: add light
    // auto light_1 = make_shared<sphere>(point3(5 * (i + 1), 2, -4), 2, make_shared<diffuse_light>(color(0.1, 0.9,
    // 0.4)));
    auto light_1 = make_shared<sphere>(point3(5 * (i + 1), 2 + 2 * std::sin(i * 0.01), -4 + +10 * std::sin(i * 0.01)),
                                       2, make_shared<diffuse_light>(nt));
    objects.add(light_1);
    // lights_list.add(light_1);
  }

  // ground
  auto pertext = make_shared<noise_texture>(10, color(.8, .4, .2));
  const auto ground_size = 10000;
  objects.add(make_shared<sphere>(point3(0, -ground_size, 0), ground_size, make_shared<lambertian>(pertext)));
  // objects.add(make_shared<sphere>(point3(0, 0, 0), ground_size, make_shared<lambertian>(pertext)));

  Scene scene;
  scene.world = objects;
  // scene.background = color(0.3, 0.03, 0.4);
  scene.background = color(0.3, 0.03, 0.4);
  scene.cam.samples_per_pixel = 5;
  scene.cam.lookfrom = point3(0, 7, 25);
  scene.cam.lookat = point3(0, 0, 0);
  scene.cam.vfov = 40.0;

  return scene;
}

}  // namespace glimpse
