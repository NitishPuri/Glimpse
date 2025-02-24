#include <functional>
#include <unordered_map>

#include "glimpse.h"

Scene random_scene() {
  hittable_list world;

  auto checker =
      make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));
  auto ground_material = make_shared<lambertian>(checker);
  world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

  for (int a = -11; a < 11; ++a) {
    for (int b = -11; b < 11; ++b) {
      auto choose_mat = random_double();
      point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

      if ((center - point3(4, 0.2, 0)).length() > 0.9) {
        shared_ptr<material> sphere_material;

        if (choose_mat < 0.9) {
          // diffuse
          auto albedo = color::random() * color::random();
          sphere_material = make_shared<lambertian>(albedo);
          auto center2 = center + vec3(0, random_double(0, 0.5), 0);
          world.add(make_shared<moving_sphere>(center, center2, 0.0, 1.0, 0.2,
                                               sphere_material));
        } else if (choose_mat < 0.95) {
          // metal
          auto albedo = color::random(0.5, 1);
          auto fuzz = random_double(0, 0.5);
          sphere_material = make_shared<metal>(albedo, fuzz);
          world.add(make_shared<sphere>(center, 0.2, sphere_material));
        } else {
          // glass
          sphere_material = make_shared<dielectric>(1.5);
          world.add(make_shared<sphere>(center, 0.2, sphere_material));
        }
      }
    }
  }

  auto material1 = make_shared<dielectric>(1.5);
  world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

  auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
  world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

  auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
  world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

  Scene scene;
  scene.background = color(0.7, 0.8, 1.0);
  scene.lookfrom = point3(13, 2, 3);
  scene.lookat = point3(0, 0, 0);
  scene.vfov = 20.0f;
  scene.aperture = 0.1f;

  return scene;
}

Scene single_sphere() {
  hittable_list objects;

  auto material = make_shared<lambertian>(color(0.8, 0.3, 0.3));
  objects.add(make_shared<sphere>(point3(0, 0, -1), 0.5, material));

  objects.add(
      make_shared<sphere>(point3(0, -100.5, -1), 100,
                          make_shared<lambertian>(color(0.8, 0.8, 0.0))));

  Scene scene;
  scene.world = objects;
  scene.background = color(0.7, 0.8, 1.0);
  scene.lookfrom = point3(13, 2, 3);
  scene.lookat = point3(0, 0, 0);
  scene.vfov = 20.0;

  return scene;
}

Scene two_spheres() {
  hittable_list objects;

  auto checker =
      make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));

  objects.add(make_shared<sphere>(point3(0, -10, 0), 10,
                                  make_shared<lambertian>(checker)));
  objects.add(make_shared<sphere>(point3(0, 10, 0), 10,
                                  make_shared<lambertian>(checker)));

  Scene scene;
  scene.world = objects;
  scene.background = color(0.7, 0.8, 1.0);
  scene.lookfrom = point3(13, 2, 3);
  scene.lookat = point3(0, 0, 0);
  scene.vfov = 20.0;

  return scene;
}

Scene two_perlin_spheres() {
  hittable_list objects;

  auto pertext = make_shared<noise_texture>(4);
  objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000,
                                  make_shared<lambertian>(pertext)));
  objects.add(make_shared<sphere>(point3(0, 2, 0), 2,
                                  make_shared<lambertian>(pertext)));

  Scene scene;
  scene.world = objects;
  scene.background = color(0.7, 0.8, 1.0);
  scene.lookfrom = point3(13, 2, 3);
  scene.lookat = point3(0, 0, 0);
  scene.vfov = 20.0;

  return scene;
}

Scene earth() {
  // auto earth_texture = make_shared<image_texture>(ROOT "/res/earthmap.jpg");
  auto earth_texture = make_shared<image_texture>("/res/earthmap.jpg");
  auto earth_surface = make_shared<lambertian>(earth_texture);
  auto globe = make_shared<sphere>(point3(0, 0, 0), 2, earth_surface);

  Scene scene;
  scene.world = hittable_list(globe);
  scene.background = color(0.7, 0.8, 1.0);
  scene.lookfrom = point3(13, 2, 3);
  scene.lookat = point3(0, 0, 0);
  scene.vfov = 20.0;

  return scene;
}

Scene simple_light() {
  hittable_list objects;

  auto pertext = make_shared<noise_texture>(4);
  objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000,
                                  make_shared<lambertian>(pertext)));
  objects.add(make_shared<sphere>(point3(0, 2, 0), 2,
                                  make_shared<lambertian>(pertext)));

  auto difflight = make_shared<diffuse_light>(color(4, 4, 4));
  objects.add(make_shared<xy_rect>(3, 5, 1, 3, -2, difflight));

  Scene scene;
  scene.world = objects;
  scene.samples_per_pixel = 400;
  scene.background = color(0, 0, 0);
  scene.lookfrom = point3(26, 3, 6);
  scene.lookat = point3(0, 2, 0);
  scene.vfov = 20.0;

  return scene;
}

Scene cornell_box() {
  hittable_list objects;

  auto red = make_shared<lambertian>(color(.65, .05, .05));
  auto white = make_shared<lambertian>(color(.73, .73, .73));
  auto green = make_shared<lambertian>(color(.12, .45, .15));
  auto light = make_shared<diffuse_light>(color(15, 15, 15));

  objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
  objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
  objects.add(make_shared<xz_rect>(213, 343, 227, 332, 554, light));
  objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
  objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
  objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

  shared_ptr<hittable> box1 =
      make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), white);
  box1 = make_shared<rotate_y>(box1, 15);
  box1 = make_shared<translate>(box1, vec3(265, 0, 295));
  objects.add(box1);

  shared_ptr<hittable> box2 =
      make_shared<box>(point3(0, 0, 0), point3(165, 165, 165), white);
  box2 = make_shared<rotate_y>(box2, -18);
  box2 = make_shared<translate>(box2, vec3(130, 0, 65));
  objects.add(box2);

  Scene scene;
  scene.world = objects;
  scene.aspect_ratio = 1.0;
  scene.image_width = 600;
  scene.samples_per_pixel = 200;
  scene.background = color(0, 0, 0);
  scene.lookfrom = point3(278, 278, -800);
  scene.lookat = point3(278, 278, 0);
  scene.vfov = 40.0;

  return scene;
}

Scene cornell_smoke() {
  hittable_list objects;

  auto red = make_shared<lambertian>(color(.65, .05, .05));
  auto white = make_shared<lambertian>(color(.73, .73, .73));
  auto green = make_shared<lambertian>(color(.12, .45, .15));
  auto light = make_shared<diffuse_light>(color(7, 7, 7));

  objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
  objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
  objects.add(make_shared<xz_rect>(113, 443, 127, 432, 554, light));
  objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
  objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
  objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

  shared_ptr<hittable> box1 =
      make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), white);
  box1 = make_shared<rotate_y>(box1, 15);
  box1 = make_shared<translate>(box1, vec3(265, 0, 295));

  shared_ptr<hittable> box2 =
      make_shared<box>(point3(0, 0, 0), point3(165, 165, 165), white);
  box2 = make_shared<rotate_y>(box2, -18);
  box2 = make_shared<translate>(box2, vec3(130, 0, 65));

  objects.add(make_shared<constant_medium>(box1, 0.01, color(0, 0, 0)));
  objects.add(make_shared<constant_medium>(box2, 0.01, color(1, 1, 1)));

  Scene scene;
  scene.world = objects;
  scene.aspect_ratio = 1.0;
  scene.image_width = 600;
  scene.samples_per_pixel = 200;
  scene.lookfrom = point3(278, 278, -800);
  scene.lookat = point3(278, 278, 0);
  scene.vfov = 40.0;

  return scene;
}

Scene final_scene() {
  hittable_list boxes1;
  auto ground = make_shared<lambertian>(color(0.48, 0.83, 0.53));

  const int boxes_per_side = 20;
  for (int i = 0; i < boxes_per_side; i++) {
    for (int j = 0; j < boxes_per_side; j++) {
      auto w = 100.0;
      auto x0 = -1000.0 + i * w;
      auto z0 = -1000.0 + j * w;
      auto y0 = 0.0;
      auto x1 = x0 + w;
      auto y1 = random_double(1, 101);
      auto z1 = z0 + w;

      boxes1.add(
          make_shared<box>(point3(x0, y0, z0), point3(x1, y1, z1), ground));
    }
  }

  hittable_list objects;

  objects.add(make_shared<bvh_node>(boxes1, 0, 1));

  auto light = make_shared<diffuse_light>(color(7, 7, 7));
  objects.add(make_shared<xz_rect>(123, 423, 147, 412, 554, light));

  auto center1 = point3(400, 400, 200);
  auto center2 = center1 + vec3(30, 0, 0);
  auto moving_sphere_material = make_shared<lambertian>(color(0.7, 0.3, 0.1));
  objects.add(make_shared<moving_sphere>(center1, center2, 0, 1, 50,
                                         moving_sphere_material));

  objects.add(make_shared<sphere>(point3(260, 150, 45), 50,
                                  make_shared<dielectric>(1.5)));
  objects.add(make_shared<sphere>(
      point3(0, 150, 145), 50, make_shared<metal>(color(0.8, 0.8, 0.9), 1.0)));

  auto boundary = make_shared<sphere>(point3(360, 150, 145), 70,
                                      make_shared<dielectric>(1.5));
  objects.add(boundary);
  objects.add(
      make_shared<constant_medium>(boundary, 0.2, color(0.2, 0.4, 0.9)));
  boundary =
      make_shared<sphere>(point3(0, 0, 0), 5000, make_shared<dielectric>(1.5));
  objects.add(make_shared<constant_medium>(boundary, .0001, color(1, 1, 1)));

  // auto emat = make_shared<lambertian>(make_shared<image_texture>(ROOT
  // "/res/earthmap.jpg"));
  auto emat =
      make_shared<lambertian>(make_shared<image_texture>("/res/earthmap.jpg"));
  objects.add(make_shared<sphere>(point3(400, 200, 400), 100, emat));
  auto pertext = make_shared<noise_texture>(0.1);
  objects.add(make_shared<sphere>(point3(220, 280, 300), 80,
                                  make_shared<lambertian>(pertext)));

  hittable_list boxes2;
  auto white = make_shared<lambertian>(color(.73, .73, .73));
  int ns = 1000;
  for (int j = 0; j < ns; j++) {
    boxes2.add(make_shared<sphere>(point3::random(0, 165), 10, white));
  }

  objects.add(make_shared<translate>(
      make_shared<rotate_y>(make_shared<bvh_node>(boxes2, 0.0, 1.0), 15),
      vec3(-100, 270, 395)));

  Scene scene;
  scene.aspect_ratio = 1.0;
  scene.image_width = 800;
  scene.samples_per_pixel = 10000;
  scene.background = color(0, 0, 0);
  scene.lookfrom = point3(478, 278, -600);
  scene.lookat = point3(278, 278, 0);
  scene.vfov = 40.0;

  return scene;
}

std::unordered_map<std::string, std::function<Scene()>> Scene::SceneMap{
    {"random_scene", []() { return random_scene(); }},
    {"two_spheres", []() { return two_spheres(); }},
    {"two_perlin_spheres", []() { return two_perlin_spheres(); }},
    {"earth", []() { return earth(); }},
    {"simple_light", []() { return simple_light(); }},
    {"cornell_box", []() { return cornell_box(); }},
    {"cornell_smoke", []() { return cornell_smoke(); }},
    {"final_scene", []() { return final_scene(); }},
    {"single_sphere", []() { return single_sphere(); }}};

// std::unordered_map<std::string, Scene> SceneMap{
//     {"random_scene", random_scene()},
//     {"two_spheres", two_spheres()},
//     {"two_perlin_spheres", two_perlin_spheres()},
//     {"earth", earth()},
//     {"simple_light", simple_light()},
//     {"cornell_box", cornell_box()},
//     {"cornell_smoke", cornell_smoke()},
//     {"final_scene", final_scene()}};

std::vector<std::string> Scene::SceneNames{
    "single_sphere",      "random_scene",  "two_spheres",
    "two_perlin_spheres", "earth",         "simple_light",
    "cornell_box",        "cornell_smoke", "final_scene"};
