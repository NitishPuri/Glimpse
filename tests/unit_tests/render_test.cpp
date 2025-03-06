#include "core/render.h"

#include "core/hittables/bvh_node.h"
#include "core/hittables/sphere.h"
#include "core/image.h"
#include "core/material.h"
#include "core/ray.h"
#include "core/vec3.h"

#define BOOST_UT_DISABLE_MODULE
#include "boost/ut.hpp"  // import boost.ut;

// TODO: This should written properly to handle multithreading,
// and then should be extended to do end to end tests by producing image results with given scenes,
// and comparing with expected results.

// Helper function to create a simple test scene
Scene create_test_scene() {
  Scene scene;

  // Setup simple camera
  scene.cam.image_width = 10;
  scene.cam.image_height = 8;
  scene.cam.sqrt_spp = 1;
  scene.cam.recip_sqrt_spp = 1.0;
  scene.cam.max_depth = 1;
  scene.cam.lookfrom = point3(0, 0, 0);
  scene.cam.lookat = point3(0, 0, -1);
  scene.cam.vup = vec3(0, 1, 0);
  scene.cam.vfov = 90.0;
  scene.cam.defocus_angle = 0.0;
  scene.cam.initialize();

  // Basic world with one sphere
  auto material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
  scene.world.add(make_shared<sphere>(point3(0, 0, -1), 0.5, material));

  // Black background
  scene.background = color(0, 0, 0);

  return scene;
}

void render_test() {
  using namespace boost::ut;

  // TODO: Fix this test
  "renderer"_test = [] {
    skip / "render_scene"_test = [] {
      // Create renderer and image
      Renderer renderer;
      Image image(10, 8);

      // Create a simple test scene
      Scene scene = create_test_scene();

      // Test basic rendering
      std::atomic<int> progress = 0;
      renderer.render_scene(scene, image, &progress);

      // here we need to wait for the rendering to finish
      // and then check the image, or maybe let that be handled by e2e tests
      // willl only check non image related stuff here, like progress and if the renderer launches stuff as expected,
      // maybe also test if the renderer can stop rendering when trigegred
      // and if the progress is updated correctly, or leave that to e2e as well?

      // I think we should test the launch,  dispatch, progress and stopping logic here,
      // and then test the rendering logic in e2e tests

      // Check that rendering produced some non-black pixels
      bool has_content = false;
      for (int j = 0; j < image.height; ++j) {
        for (int i = 0; i < image.width; ++i) {
          ImageColor pixel = image.get(i, j);
          if (pixel.rgb[0] > 0 || pixel.rgb[1] > 0 || pixel.rgb[2] > 0) {
            has_content = true;
            break;
          }
        }
        if (has_content) break;
      }
      expect(has_content) << "Rendered image should contain some non-black pixels";

      // Progress should have been updated
      expect(progress.load() > 0_i) << "Progress counter should be incremented during rendering";
    };

    // TODO: Fix this test
    skip / "ray_color"_test = [] {
      // Test the ray_color function directly
      hittable_list world;
      auto material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
      world.add(make_shared<sphere>(point3(0, 0, -1), 0.5, material));

      auto bvh = bvh_node(world);
      hittable_list lights;
      color background(0, 0, 0);

      // Ray that hits the sphere
      ray r(point3(0, 0, 0), vec3(0, 0, -1));
      color c1 = ray_color(r, background, bvh, 1, lights, false);
      expect(c1.x() > 0.0_d) << "Ray hitting object should return non-zero color";

      // Ray that misses everything
      ray r2(point3(0, 0, 0), vec3(0, 1, 0));
      color c2 = ray_color(r2, background, bvh, 1, lights, false);
      expect(c2 == background) << "Ray missing all objects should return background color";
    };

    "sample_square_stratified"_test = [] {
      // Test stratified sampling
      vec3 sample = sample_square_stratified(0, 0, 1.0);
      expect(sample.x() >= -0.5_d && sample.x() <= 0.5_d) << "Sample x should be in range [-0.5, 0.5]";
      expect(sample.y() >= -0.5_d && sample.y() <= 0.5_d) << "Sample y should be in range [-0.5, 0.5]";
      expect(sample.z() == 0.0_d) << "Sample z should be 0";
    };

    "stop_rendering"_test = [] {
      // Test the stop rendering flag
      Renderer::stop_rendering = true;
      expect(Renderer::stop_rendering.load() == true);

      Renderer::stop_rendering = false;
      expect(Renderer::stop_rendering.load() == false);
    };
  };
}
