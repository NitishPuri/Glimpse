#include "core/render.h"

#include "core/hittables/bvh_node.h"
#include "core/hittables/sphere.h"
#include "core/image.h"
#include "core/material.h"
#include "core/ray.h"
#include "core/vec3.h"

//
#include "../test_cfg.h"

using namespace glimpse;

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
    // Regression (2026-10-10): Glimpse_cli never called cam.initialize(), so image_height stayed 0 and
    // render_scene produced an all-black image. render_scene must initialise the camera itself.
    "render_scene_initializes_camera"_test = [] {
      Scene scene;
      scene.cam.aspect_ratio = 2.0;
      scene.cam.image_width = 8;  // image_height deliberately left at 0
      scene.cam.samples_per_pixel = 1;
      scene.cam.max_depth = 1;
      scene.cam.defocus_angle = 0.0f;
      scene.background = color(1, 1, 1);  // every camera ray misses, so every pixel is white
      // One object behind the camera, because the BVH needs at least one primitive.
      scene.world.add(make_shared<sphere>(point3(0, 0, 100), 0.1, make_shared<lambertian>(color(0.5, 0.5, 0.5))));

      Image image(8, 4);
      Renderer renderer;
      renderer.render_scene(scene, image);

      int lit = 0;
      for (int j = 0; j < image.height; ++j)
        for (int i = 0; i < image.width; ++i)
          if (image.get(i, j).rgb[0] > 0) ++lit;
      expect(lit == 32_i) << "every pixel should see the white background";
    };

    // Image rows are top-down like image files (row 0 = top of the picture); the camera's v runs bottom-up.
    // Regression (2026-10-10): the CLI wrote every image upside down.
    "image_rows_are_top_down"_test = [] {
      Scene scene;
      scene.cam.aspect_ratio = 1.0;
      scene.cam.image_width = 16;
      scene.cam.samples_per_pixel = 1;
      scene.cam.max_depth = 1;
      scene.cam.vfov = 90.0f;
      scene.cam.defocus_angle = 0.0f;
      scene.background = color(0, 0, 0);
      // A light entirely in the upper half of the view.
      scene.world.add(make_shared<sphere>(point3(0, 0.5, -1), 0.3, make_shared<diffuse_light>(color(1, 1, 1))));

      Image image(16, 16);
      Renderer renderer;
      renderer.render_scene(scene, image);

      long top = 0, bottom = 0;
      for (int j = 0; j < image.height; ++j)
        for (int i = 0; i < image.width; ++i) (j < image.height / 2 ? top : bottom) += image.get(i, j).rgb[0];
      expect(top > 0 and bottom == 0) << "light must land in the top rows: top=" << top << " bottom=" << bottom;
    };

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

    "ray_color"_test = [] {
      // Test the ray_color function directly
      hittable_list world;
      auto material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
      world.add(make_shared<sphere>(point3(0, 0, -1), 0.5, material));

      auto bvh = bvh_node(world);
      hittable_list lights;
      color background(0.1, 0.1, 0.1);

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

    // The stop flag must also abort capped (fixed-spp) renders, so closing the viewer doesn't wait for a long
    // render to finish. render_scene clears the flag on the way out, ready for the next render.
    "stop_flag_aborts_capped_render"_test = [] {
      Scene scene;
      scene.cam.aspect_ratio = 1.0;
      scene.cam.image_width = 16;
      scene.cam.samples_per_pixel = 4;
      scene.cam.max_depth = 1;
      scene.background = color(1, 1, 1);  // would light every pixel if anything rendered
      scene.world.add(make_shared<sphere>(point3(0, 0, 100), 0.1, make_shared<lambertian>(color(0.5, 0.5, 0.5))));

      Image image(16, 16);
      Renderer renderer;
      Renderer::stop_rendering = true;  // already set: no pixel should be rendered
      renderer.render_scene(scene, image);

      int lit = 0;
      for (int j = 0; j < image.height; ++j)
        for (int i = 0; i < image.width; ++i)
          if (image.get(i, j).rgb[0] > 0) ++lit;
      expect(lit == 0_i) << "a stopped render writes nothing";
      expect(!Renderer::stop_rendering.load()) << "the flag is cleared after the render returns";
    };
  };
}
