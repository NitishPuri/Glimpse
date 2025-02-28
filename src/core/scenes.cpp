#include <functional>
#include <unordered_map>

#include "glimpse.h"

Scene random_scene() {
  hittable_list world;

  auto checker = make_shared<checker_texture>(1, color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));
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
          world.add(make_shared<moving_sphere>(center, center2, 0.2, sphere_material));
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
  scene.world = world;
  scene.background = color(0.7, 0.8, 1.0);
  scene.cam.lookfrom = point3(13, 2, 3);
  scene.cam.lookat = point3(0, 0, 0);
  scene.cam.vfov = 20.0f;
  scene.cam.defocus_angle = 0.1f;

  return scene;
}

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

Scene two_spheres_check() {
  hittable_list objects;

  auto checker = make_shared<checker_texture>(1, color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));

  objects.add(make_shared<sphere>(point3(0, -10, 0), 10, make_shared<lambertian>(checker)));
  objects.add(make_shared<sphere>(point3(0, 10, 0), 10, make_shared<lambertian>(checker)));

  Scene scene;
  scene.world = objects;
  scene.background = color(0.7, 0.8, 1.0);
  scene.cam.lookfrom = point3(13, 2, 3);
  scene.cam.lookat = point3(0, 0, 0);
  scene.cam.vfov = 20.0;

  return scene;
}

Scene two_perlin_spheres() {
  hittable_list objects;

  auto pertext = make_shared<noise_texture>(4);
  objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
  objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

  Scene scene;
  scene.world = objects;
  scene.background = color(0.7, 0.8, 1.0);
  scene.cam.lookfrom = point3(13, 2, 3);
  scene.cam.lookat = point3(0, 0, 0);
  scene.cam.vfov = 20.0;

  return scene;
}

Scene earth() {
  auto earth_texture = make_shared<image_texture>("./res/earthmap.jpg");
  auto earth_surface = make_shared<lambertian>(earth_texture);
  auto globe = make_shared<sphere>(point3(0, 0, 0), 2, earth_surface);

  Scene scene;
  scene.world = hittable_list(globe);
  scene.background = color(0.7, 0.8, 1.0);
  scene.cam.lookfrom = point3(13, 2, 3);
  scene.cam.lookat = point3(0, 0, 0);
  scene.cam.vfov = 20.0;

  return scene;
}

Scene simple_light() {
  hittable_list objects;

  auto pertext = make_shared<noise_texture>(4);
  objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
  objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

  auto difflight = make_shared<diffuse_light>(color(4, 4, 4));
  auto sphere_light = make_shared<sphere>(point3(0, 7, 0), 2, difflight);
  auto quad_light = make_shared<quad>(point3{3, 1, -2}, vec3{2, 0, 0}, vec3{0, 2, 0}, difflight);
  objects.add(sphere_light);
  objects.add(quad_light);

  hittable_list lights_list;
  lights_list.add(sphere_light);
  lights_list.add(quad_light);

  Scene scene;
  scene.world = objects;
  scene.lights = lights_list;
  scene.cam.samples_per_pixel = 400;
  scene.background = color(0, 0, 0);
  scene.cam.lookfrom = point3(26, 3, 6);
  scene.cam.lookat = point3(0, 2, 0);
  scene.cam.vfov = 20.0;

  return scene;
}

Scene cornell_box() {
  hittable_list world;

  auto red = make_shared<lambertian>(color(.65, .05, .05));
  auto white = make_shared<lambertian>(color(.73, .73, .73));
  auto green = make_shared<lambertian>(color(.12, .45, .15));
  auto light = make_shared<diffuse_light>(color(15, 15, 15));

  world.add(make_shared<quad>(point3(555, 0, 0), vec3(0, 555, 0), vec3(0, 0, 555), green));

  world.add(make_shared<quad>(point3(0, 0, 0), vec3(0, 555, 0), vec3(0, 0, 555), red));
  world.add(make_shared<quad>(point3(343, 554, 332), vec3(-130, 0, 0), vec3(0, 0, -105), light));

  world.add(make_shared<quad>(point3(0, 0, 0), vec3(555, 0, 0), vec3(0, 0, 555), white));
  world.add(make_shared<quad>(point3(555, 555, 555), vec3(-555, 0, 0), vec3(0, 0, -555), white));
  world.add(make_shared<quad>(point3(0, 0, 555), vec3(555, 0, 0), vec3(0, 555, 0), white));

  // shared_ptr<material> aluminium = make_shared<metal>(color(0.8, 0.85, 0.88), 0.0);
  // shared_ptr<hittable> box1 = box(point3(0, 0, 0), point3(165, 330, 165), aluminium);
  shared_ptr<hittable> box1 = box(point3(0, 0, 0), point3(165, 330, 165), white);
  box1 = make_shared<rotate_y>(box1, 15);
  box1 = make_shared<translate>(box1, vec3(265, 0, 295));
  world.add(box1);

  // Glass Sphere
  // auto glass = make_shared<dielectric>(1.5);
  // world.add(make_shared<sphere>(point3(190, 90, 190), 90, glass));

  shared_ptr<hittable> box2 = box(point3(0, 0, 0), point3(165, 165, 165), white);
  box2 = make_shared<rotate_y>(box2, -18);
  box2 = make_shared<translate>(box2, vec3(130, 0, 65));
  world.add(box2);

  // lights
  auto empty_material = shared_ptr<material>();
  quad light_quad(point3(343, 554, 332), vec3(-130, 0, 0), vec3(0, 0, -105), empty_material);
  hittable_list lights_list;
  lights_list.add(make_shared<quad>(light_quad));

  Scene scene;
  scene.world = world;
  scene.lights = lights_list;
  scene.cam.aspect_ratio = 1.0;
  scene.cam.image_width = 600;
  scene.cam.samples_per_pixel = 100;
  scene.background = color(0, 0, 0);
  scene.cam.lookfrom = point3(278, 278, -800);
  scene.cam.lookat = point3(278, 278, 0);
  scene.cam.vfov = 40.0;

  return scene;
}

Scene cornell_smoke() {
  hittable_list objects;

  auto red = make_shared<lambertian>(color(.65, .05, .05));
  auto white = make_shared<lambertian>(color(.73, .73, .73));
  auto green = make_shared<lambertian>(color(.12, .45, .15));
  auto light = make_shared<diffuse_light>(color(15, 15, 15));

  objects.add(make_shared<quad>(point3(555, 0, 0), vec3(0, 555, 0), vec3(0, 0, 555), green));
  objects.add(make_shared<quad>(point3(0, 0, 0), vec3(0, 555, 0), vec3(0, 0, 555), red));
  objects.add(make_shared<quad>(point3(113, 554, 127), vec3(330, 0, 0), vec3(0, 0, 305), light));
  objects.add(make_shared<quad>(point3(0, 555, 0), vec3(555, 0, 0), vec3(0, 0, 555), white));
  objects.add(make_shared<quad>(point3(0, 0, 0), vec3(555, 0, 0), vec3(0, 0, 555), white));
  objects.add(make_shared<quad>(point3(0, 0, 555), vec3(555, 0, 0), vec3(0, 555, 0), white));

  shared_ptr<hittable> box1 = box(point3(0, 0, 0), point3(165, 330, 165), white);
  box1 = make_shared<rotate_y>(box1, 15);
  box1 = make_shared<translate>(box1, vec3(265, 0, 295));
  // objects.add(box1);

  shared_ptr<hittable> box2 = box(point3(0, 0, 0), point3(165, 165, 165), white);
  box2 = make_shared<rotate_y>(box2, -18);
  box2 = make_shared<translate>(box2, vec3(130, 0, 65));
  // objects.add(box2);

  objects.add(make_shared<constant_medium>(box1, 0.01, color(0, 0, 0)));
  objects.add(make_shared<constant_medium>(box2, 0.01, color(1, 1, 1)));

  // lights
  auto empty_material = shared_ptr<material>();
  quad light_quad(point3(113, 554, 127), vec3(330, 0, 0), vec3(0, 0, 305), empty_material);
  hittable_list lights_list;
  lights_list.add(make_shared<quad>(light_quad));

  Scene scene;
  scene.world = objects;
  scene.lights = lights_list;
  scene.cam.aspect_ratio = 1.0;
  scene.cam.image_width = 600;
  scene.cam.samples_per_pixel = 1000;
  scene.background = color(0, 0, 0);
  scene.cam.lookfrom = point3(278, 278, -800);
  scene.cam.lookat = point3(278, 278, 0);
  scene.cam.vfov = 40.0;

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

      boxes1.add(box(point3(x0, y0, z0), point3(x1, y1, z1), ground));
    }
  }

  // std::cout << "Final scene,.. \n" << std::endl;

  hittable_list world;

  world.add(make_shared<bvh_node>(boxes1));

  auto light = make_shared<diffuse_light>(color(7, 7, 7));
  world.add(make_shared<quad>(point3(123, 554, 147), vec3(300, 0, 0), vec3(0, 0, 265), light));
  // objects.add(make_shared<xz_rect>(123, 423, 147, 412, 554, light));

  auto center1 = point3(400, 400, 200);
  auto center2 = center1 + vec3(30, 0, 0);
  auto moving_sphere_material = make_shared<lambertian>(color(0.7, 0.3, 0.1));
  world.add(make_shared<moving_sphere>(center1, center2, 50, moving_sphere_material));

  world.add(make_shared<sphere>(point3(260, 150, 45), 50, make_shared<dielectric>(1.5)));
  world.add(make_shared<sphere>(point3(0, 150, 145), 50, make_shared<metal>(color(0.8, 0.8, 0.9), 1.0)));

  auto boundary = make_shared<sphere>(point3(360, 150, 145), 70, make_shared<dielectric>(1.5));
  world.add(boundary);
  world.add(make_shared<constant_medium>(boundary, 0.2, color(0.2, 0.4, 0.9)));
  boundary = make_shared<sphere>(point3(0, 0, 0), 5000, make_shared<dielectric>(1.5));
  world.add(make_shared<constant_medium>(boundary, .0001, color(1, 1, 1)));

  auto emat = make_shared<lambertian>(make_shared<image_texture>("./res/earthmap.jpg"));
  world.add(make_shared<sphere>(point3(400, 200, 400), 100, emat));
  auto pertext = make_shared<noise_texture>(0.1);
  world.add(make_shared<sphere>(point3(220, 280, 300), 80, make_shared<lambertian>(pertext)));

  hittable_list boxes2;
  auto white = make_shared<lambertian>(color(.73, .73, .73));
  int ns = 1000;
  for (int j = 0; j < ns; j++) {
    boxes2.add(make_shared<sphere>(point3::random(0, 165), 10, white));
  }

  world.add(make_shared<translate>(make_shared<rotate_y>(make_shared<bvh_node>(boxes2), 15), vec3(-100, 270, 395)));

  // lights
  auto empty_material = shared_ptr<material>();
  hittable_list lights_list;
  lights_list.add(make_shared<quad>(point3(123, 554, 147), vec3(300, 0, 0), vec3(0, 0, 265), empty_material));

  Scene scene;
  scene.world = world;
  scene.cam.aspect_ratio = 1.0;
  scene.cam.image_width = 800;
  scene.cam.samples_per_pixel = 1000;
  scene.background = color(0, 0, 0);
  scene.cam.lookfrom = point3(478, 278, -600);
  scene.cam.lookat = point3(278, 278, 0);
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

Scene quads() {
  hittable_list world;

  // Materials
  auto left_red = make_shared<lambertian>(color(1.0, 0.2, 0.2));
  auto back_green = make_shared<lambertian>(color(0.2, 1.0, 0.2));
  auto right_blue = make_shared<lambertian>(color(0.2, 0.2, 1.0));
  auto upper_orange = make_shared<lambertian>(color(1.0, 0.5, 0.0));
  auto lower_teal = make_shared<lambertian>(color(0.2, 0.8, 0.8));

  // Quads
  world.add(make_shared<quad>(point3(-3, -2, 5), vec3(0, 0, -4), vec3(0, 4, 0), left_red));
  world.add(make_shared<quad>(point3(-2, -2, 0), vec3(4, 0, 0), vec3(0, 4, 0), back_green));
  world.add(make_shared<quad>(point3(3, -2, 1), vec3(0, 0, 4), vec3(0, 4, 0), right_blue));
  world.add(make_shared<quad>(point3(-2, 3, 1), vec3(4, 0, 0), vec3(0, 0, 4), upper_orange));
  world.add(make_shared<quad>(point3(-2, -3, 5), vec3(4, 0, 0), vec3(0, 0, -4), lower_teal));

  Scene scene;
  scene.world = world;
  scene.background = color(0.7, 0.8, 1.0);
  scene.cam.aspect_ratio = 1.0;
  scene.cam.image_width = 400;
  scene.cam.samples_per_pixel = 100;
  scene.cam.max_depth = 50;

  scene.cam.vfov = 80;
  scene.cam.lookfrom = point3(0, 0, 9);
  scene.cam.lookat = point3(0, 0, 0);
  scene.cam.vup = vec3(0, 1, 0);

  scene.cam.defocus_angle = 0;

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
  auto giant_mirror = make_shared<quad>(point3{-20, 1, -14}, vec3{200, 0, -100}, vec3{0, 20, 0}, aluminium);
  objects.add(giant_mirror);

  for (int i = 0; i < 13; ++i) {
    color c = color::random();
    // TODO: add light
    // auto light_1 = make_shared<sphere>(point3(5 * (i + 1), 2, -4), 2, make_shared<diffuse_light>(color(0.1, 0.9,
    // 0.4)));
    auto light_1 = make_shared<sphere>(point3(5 * (i + 1), 2, -4), 2, make_shared<diffuse_light>(c));
    objects.add(light_1);
    lights_list.add(light_1);
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

std::unordered_map<std::string, std::function<Scene()>> Scene::SceneMap = {
    {"random_scene", []() { return random_scene(); }},
    {"two_diffuse_spheres", []() { return two_diffuse_spheres(); }},
    {"two_spheres_check", []() { return two_spheres_check(); }},
    {"two_perlin_spheres", []() { return two_perlin_spheres(); }},
    {"earth", []() { return earth(); }},
    {"simple_light", []() { return simple_light(); }},
    {"cornell_box", []() { return cornell_box(); }},
    {"cornell_smoke", []() { return cornell_smoke(); }},
    {"final_scene", []() { return final_scene(); }},
    {"directions_test", []() { return directions_test(); }},
    {"material_showcase", []() { return material_showcase(); }},
    {"quads", []() { return quads(); }},
    {"untitled_1", []() { return untitled_1(); }},
};

std::vector<std::string> Scene::SceneNames = {"directions_test",    "two_diffuse_spheres",
                                              "two_spheres_check",  "material_showcase",
                                              "untitled_1",         "earth",
                                              "two_perlin_spheres", "quads",
                                              "simple_light",       "cornell_box",
                                              "random_scene",       "cornell_smoke",
                                              "final_scene"};
