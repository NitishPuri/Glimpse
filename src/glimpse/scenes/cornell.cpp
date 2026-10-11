// The Cornell box and its smoke variant (Ray Tracing: The Next Week).

#include "glimpse/glimpse.h"
#include "glimpse/scenes/factories.h"

namespace glimpse {

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

}  // namespace glimpse
