#include "glimpse/convergence.h"

#include <fstream>
#include <sstream>
#include <vector>

#include "glimpse/materials.h"
#include "glimpse/scene.h"
#include "glimpse/shapes.h"
#include "glimpse/util/float_image.h"

//
#include "../test_cfg.h"

using namespace glimpse;

namespace {
// Every camera ray misses and sees `background`, so each render is exact at any spp.
Scene backgroundOnlyScene(Color background) {
  Scene scene;
  scene.cam.aspectRatio = 2.0;
  scene.cam.imageWidth = 8;
  scene.cam.maxDepth = 1;
  scene.cam.defocusAngle = 0.0f;
  scene.background = background;
  // One object behind the camera, because the BVH needs at least one primitive.
  scene.world.add(make_shared<Sphere>(Point3(0, 0, 100), 0.1, make_shared<Lambertian>(Color(0.5, 0.5, 0.5))));
  return scene;
}

FloatImage constantImage(int w, int h, float value) {
  FloatImage image(w, h);
  for (float& c : image.rgb) c = value;
  return image;
}
}  // namespace

static ut::suite<"convergence"> convergenceSuite = [] {
  using namespace boost::ut;

  "convergence"_test = [] {
    "schedule_roughly_doubles"_test = [] {
      expect(convergenceSchedule(1024) == std::vector<int>{1, 4, 9, 16, 36, 64, 121, 256, 529, 1024});
    };

    "schedule_ends_at_largest_square"_test = [] {
      expect(convergenceSchedule(1) == std::vector<int>{1});
      expect(convergenceSchedule(10) == std::vector<int>{1, 4, 9}) << "10 spp renders as 3x3, like --spp";
      expect(convergenceSchedule(100) == std::vector<int>{1, 4, 9, 16, 36, 64, 100})
          << "10x10 isn't on the sqrt(2) ladder, but the requested maximum is always the last point";
    };

    "measure_against_reference"_test = [] {
      const Scene scene = backgroundOnlyScene(Color(0.5, 0.5, 0.5));
      std::vector<int> seen;
      const auto points = measureConvergence(scene, constantImage(8, 4, 0.5f), {1, 4},
                                             [&](const ConvergencePoint& p) { seen.push_back(p.spp); });
      expect(points.size() == 2_u);
      expect(seen == std::vector<int>{1, 4}) << "callback once per render, in order";
      for (const auto& p : points) {
        expect(p.error.mse < 1e-12) << "a background-only render is exact";
        expect(p.seconds >= 0.0);
      }

      const auto off = measureConvergence(scene, constantImage(8, 4, 1.0f), {1});
      expect(off.size() == 1_u);
      expect(std::abs(off[0].error.mse - 0.25) < 1e-6) << "0.5 vs 1.0 in every channel";
    };

    "size_mismatch_stops"_test = [] {
      const auto points = measureConvergence(backgroundOnlyScene(Color(0, 0, 0)), constantImage(5, 5, 0.f), {1, 4});
      expect(points.empty());
    };

    "csv"_test = [] {
      const std::string path = testOutputPath("convergence.csv");
      ErrorStats e;
      e.mse = 0.25;
      e.rmse = 0.5;
      e.relMSE = 2;
      expect(writeConvergenceCSV(path, {{4, 1.5, e}}));
      std::ifstream in(path);
      std::stringstream text;
      text << in.rdbuf();
      expect(text.str() == "spp,seconds,mse,rmse,relmse\n4,1.5,0.25,0.5,2\n") << text.str();
    };
  };
};
