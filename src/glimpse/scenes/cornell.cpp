// The Cornell box and its smoke variant (Ray Tracing: The Next Week).

#include "glimpse/glimpse.h"
#include "glimpse/scenes/factories.h"

namespace glimpse {

Scene cornellBox() {
  HittableList world;

  auto red = make_shared<Lambertian>(Color(.65, .05, .05));
  auto white = make_shared<Lambertian>(Color(.73, .73, .73));
  auto green = make_shared<Lambertian>(Color(.12, .45, .15));
  auto light = make_shared<DiffuseLight>(Color(15, 15, 15));

  world.add(make_shared<Quad>(Point3(555, 0, 0), Vec3(0, 555, 0), Vec3(0, 0, 555), green));

  world.add(make_shared<Quad>(Point3(0, 0, 0), Vec3(0, 555, 0), Vec3(0, 0, 555), red));
  world.add(make_shared<Quad>(Point3(343, 554, 332), Vec3(-130, 0, 0), Vec3(0, 0, -105), light));

  world.add(make_shared<Quad>(Point3(0, 0, 0), Vec3(555, 0, 0), Vec3(0, 0, 555), white));
  world.add(make_shared<Quad>(Point3(555, 555, 555), Vec3(-555, 0, 0), Vec3(0, 0, -555), white));
  world.add(make_shared<Quad>(Point3(0, 0, 555), Vec3(555, 0, 0), Vec3(0, 555, 0), white));

  // shared_ptr<material> aluminium = make_shared<metal>(color(0.8, 0.85, 0.88), 0.0);
  // shared_ptr<hittable> box1 = box(point3(0, 0, 0), point3(165, 330, 165), aluminium);
  shared_ptr<Hittable> box1 = box(Point3(0, 0, 0), Point3(165, 330, 165), white);
  box1 = make_shared<RotateY>(box1, 15);
  box1 = make_shared<Translate>(box1, Vec3(265, 0, 295));
  world.add(box1);

  // Glass Sphere
  // auto glass = make_shared<dielectric>(1.5);
  // world.add(make_shared<sphere>(point3(190, 90, 190), 90, glass));

  shared_ptr<Hittable> box2 = box(Point3(0, 0, 0), Point3(165, 165, 165), white);
  box2 = make_shared<RotateY>(box2, -18);
  box2 = make_shared<Translate>(box2, Vec3(130, 0, 65));
  world.add(box2);

  // lights
  auto emptyMaterial = shared_ptr<Material>();
  Quad lightQuad(Point3(343, 554, 332), Vec3(-130, 0, 0), Vec3(0, 0, -105), emptyMaterial);
  HittableList lightsList;
  lightsList.add(make_shared<Quad>(lightQuad));

  Scene scene;
  scene.world = world;
  scene.lights = lightsList;
  scene.cam.aspectRatio = 1.0;
  scene.cam.imageWidth = 600;
  scene.cam.samplesPerPixel = 100;
  scene.background = Color(0, 0, 0);
  scene.cam.lookfrom = Point3(278, 278, -800);
  scene.cam.lookat = Point3(278, 278, 0);
  scene.cam.vfov = 40.0;

  return scene;
}

Scene cornellSmoke() {
  HittableList objects;

  auto red = make_shared<Lambertian>(Color(.65, .05, .05));
  auto white = make_shared<Lambertian>(Color(.73, .73, .73));
  auto green = make_shared<Lambertian>(Color(.12, .45, .15));
  auto light = make_shared<DiffuseLight>(Color(15, 15, 15));

  objects.add(make_shared<Quad>(Point3(555, 0, 0), Vec3(0, 555, 0), Vec3(0, 0, 555), green));
  objects.add(make_shared<Quad>(Point3(0, 0, 0), Vec3(0, 555, 0), Vec3(0, 0, 555), red));
  objects.add(make_shared<Quad>(Point3(113, 554, 127), Vec3(330, 0, 0), Vec3(0, 0, 305), light));
  objects.add(make_shared<Quad>(Point3(0, 555, 0), Vec3(555, 0, 0), Vec3(0, 0, 555), white));
  objects.add(make_shared<Quad>(Point3(0, 0, 0), Vec3(555, 0, 0), Vec3(0, 0, 555), white));
  objects.add(make_shared<Quad>(Point3(0, 0, 555), Vec3(555, 0, 0), Vec3(0, 555, 0), white));

  shared_ptr<Hittable> box1 = box(Point3(0, 0, 0), Point3(165, 330, 165), white);
  box1 = make_shared<RotateY>(box1, 15);
  box1 = make_shared<Translate>(box1, Vec3(265, 0, 295));
  // objects.add(box1);

  shared_ptr<Hittable> box2 = box(Point3(0, 0, 0), Point3(165, 165, 165), white);
  box2 = make_shared<RotateY>(box2, -18);
  box2 = make_shared<Translate>(box2, Vec3(130, 0, 65));
  // objects.add(box2);

  objects.add(make_shared<ConstantMedium>(box1, 0.01, Color(0, 0, 0)));
  objects.add(make_shared<ConstantMedium>(box2, 0.01, Color(1, 1, 1)));

  // lights
  auto emptyMaterial = shared_ptr<Material>();
  Quad lightQuad(Point3(113, 554, 127), Vec3(330, 0, 0), Vec3(0, 0, 305), emptyMaterial);
  HittableList lightsList;
  lightsList.add(make_shared<Quad>(lightQuad));

  Scene scene;
  scene.world = objects;
  scene.lights = lightsList;
  scene.cam.aspectRatio = 1.0;
  scene.cam.imageWidth = 600;
  scene.cam.samplesPerPixel = 1000;
  scene.background = Color(0, 0, 0);
  scene.cam.lookfrom = Point3(278, 278, -800);
  scene.cam.lookat = Point3(278, 278, 0);
  scene.cam.vfov = 40.0;

  return scene;
}

}  // namespace glimpse
