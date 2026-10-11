#include "glimpse/render.h"

#include "glimpse/aggregates.h"
#include "glimpse/shapes.h"
#include "glimpse/util/image.h"
#include "glimpse/materials.h"
#include "glimpse/ray.h"
#include "glimpse/util/vecmath.h"

//
#include "../test_cfg.h"

using namespace glimpse;

static ut::suite<"render"> renderSuite = [] {
  using namespace boost::ut;

  "renderer"_test = [] {
    // Regression (2026-10-10): Glimpse_cli never called cam.initialize(), so imageHeight stayed 0 and
    // renderScene produced an all-black image. renderScene must initialise the camera itself.
    "render_scene_initializes_camera"_test = [] {
      Scene scene;
      scene.cam.aspectRatio = 2.0;
      scene.cam.imageWidth = 8;  // imageHeight deliberately left at 0
      scene.cam.samplesPerPixel = 1;
      scene.cam.maxDepth = 1;
      scene.cam.defocusAngle = 0.0f;
      scene.background = Color(1, 1, 1);  // every camera ray misses, so every pixel is white
      // One object behind the camera, because the BVH needs at least one primitive.
      scene.world.add(make_shared<Sphere>(Point3(0, 0, 100), 0.1, make_shared<Lambertian>(Color(0.5, 0.5, 0.5))));

      Image image(8, 4);
      Renderer renderer;
      renderer.renderScene(scene, image);

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
      scene.cam.aspectRatio = 1.0;
      scene.cam.imageWidth = 16;
      scene.cam.samplesPerPixel = 1;
      scene.cam.maxDepth = 1;
      scene.cam.vfov = 90.0f;
      scene.cam.defocusAngle = 0.0f;
      scene.background = Color(0, 0, 0);
      // A light entirely in the upper half of the view.
      scene.world.add(make_shared<Sphere>(Point3(0, 0.5, -1), 0.3, make_shared<DiffuseLight>(Color(1, 1, 1))));

      Image image(16, 16);
      Renderer renderer;
      renderer.renderScene(scene, image);

      long top = 0, bottom = 0;
      for (int j = 0; j < image.height; ++j)
        for (int i = 0; i < image.width; ++i) (j < image.height / 2 ? top : bottom) += image.get(i, j).rgb[0];
      expect(top > 0 and bottom == 0) << "light must land in the top rows: top=" << top << " bottom=" << bottom;
    };

    "ray_color"_test = [] {
      // Test the rayColor function directly
      HittableList world;
      auto mat = make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
      world.add(make_shared<Sphere>(Point3(0, 0, -1), 0.5, mat));

      auto bvh = BVHNode(world);
      HittableList lights;
      Color background(0.1, 0.1, 0.1);

      // Ray that hits the sphere
      Ray r(Point3(0, 0, 0), Vec3(0, 0, -1));
      Color c1 = rayColor(r, background, bvh, 1, lights, false);
      expect(c1.x() > 0.0_d) << "Ray hitting object should return non-zero color";

      // Ray that misses everything
      Ray r2(Point3(0, 0, 0), Vec3(0, 1, 0));
      Color c2 = rayColor(r2, background, bvh, 1, lights, false);
      expect(c2 == background) << "Ray missing all objects should return background color";
    };

    "sample_square_stratified"_test = [] {
      // Test stratified sampling
      Vec3 sample = sampleSquareStratified(0, 0, 1.0);
      expect(sample.x() >= -0.5_d && sample.x() <= 0.5_d) << "Sample x should be in range [-0.5, 0.5]";
      expect(sample.y() >= -0.5_d && sample.y() <= 0.5_d) << "Sample y should be in range [-0.5, 0.5]";
      expect(sample.z() == 0.0_d) << "Sample z should be 0";
    };

    // The stop flag must also abort capped (fixed-spp) renders, so closing the viewer doesn't wait for a long
    // render to finish. renderScene clears the flag on the way out, ready for the next render.
    "stop_flag_aborts_capped_render"_test = [] {
      Scene scene;
      scene.cam.aspectRatio = 1.0;
      scene.cam.imageWidth = 16;
      scene.cam.samplesPerPixel = 4;
      scene.cam.maxDepth = 1;
      scene.background = Color(1, 1, 1);  // would light every pixel if anything rendered
      scene.world.add(make_shared<Sphere>(Point3(0, 0, 100), 0.1, make_shared<Lambertian>(Color(0.5, 0.5, 0.5))));

      Image image(16, 16);
      Renderer renderer;
      Renderer::stopRequested = true;  // already set: no pixel should be rendered
      renderer.renderScene(scene, image);

      int lit = 0;
      for (int j = 0; j < image.height; ++j)
        for (int i = 0; i < image.width; ++i)
          if (image.get(i, j).rgb[0] > 0) ++lit;
      expect(lit == 0_i) << "a stopped render writes nothing";
      expect(!Renderer::stopRequested.load()) << "the flag is cleared after the render returns";
    };
  };
};
