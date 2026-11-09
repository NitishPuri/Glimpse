#include "gui/error_tracker.h"

#include <filesystem>

//
#include "../test_cfg.h"

using namespace glimpse;
using namespace glimpse::gui;

namespace {
// A 4x2 scene with its reference written where loadReference looks: refs/<name>.pfm, relative to the working
// directory. Tests run with build/<cfg>/test_output as the working directory so the repo's refs/ isn't touched.
Scene smallScene() {
  Scene scene;
  scene.cam.aspectRatio = 2.0;
  scene.cam.imageWidth = 4;
  scene.background = Color(0.5, 0.5, 0.5);
  return scene;
}

FloatImage constantImage(int w, int h, float value) {
  FloatImage image(w, h);
  for (float& c : image.rgb) c = value;
  return image;
}

struct InTestOutputDir {
  std::filesystem::path previous = std::filesystem::current_path();
  InTestOutputDir() {
    std::filesystem::create_directories(testOutputPath("refs"));
    std::filesystem::current_path(testOutputPath(""));
  }
  ~InTestOutputDir() { std::filesystem::current_path(previous); }
};
}  // namespace

static ut::suite<"gui_error_tracker"> guiErrorTrackerSuite = [] {
  using namespace boost::ut;

  "gui_error_tracker"_test = [] {
    InTestOutputDir cwd;
    writePFM(referenceOutputPath("tracker_scene"), constantImage(4, 2, 0.5f));

    "no_reference"_test = [] {
      ErrorTracker tracker;
      tracker.loadReference("no_such_scene", smallScene());
      expect(!tracker.ref().has_value());
      expect(tracker.unavailableReason(smallScene()).contains("no reference"));
    };

    "valid_only_for_the_reference_image"_test = [] {
      const Scene scene = smallScene();
      ErrorTracker tracker;
      tracker.loadReference("tracker_scene", scene);
      expect(tracker.unavailableReason(scene).empty()) << tracker.unavailableReason(scene);

      Scene edited = scene;
      edited.cam.imageWidth = 8;
      expect(tracker.unavailableReason(edited).contains("size"));
      edited = scene;
      edited.cam.lookfrom = Point3(1, 0, 0);
      expect(tracker.unavailableReason(edited).contains("camera"));
      edited = scene;
      edited.cam.maxDepth = 2;
      expect(tracker.unavailableReason(edited).contains("depth"));
      edited = scene;
      edited.background = Color(0, 0, 0);
      expect(tracker.unavailableReason(edited).contains("background"));
    };

    "samples_the_film"_test = [] {
      ErrorTracker tracker;
      tracker.loadReference("tracker_scene", smallScene());
      Film film(4, 2);
      for (int j = 0; j < 2; ++j)
        for (int i = 0; i < 4; ++i) {
          film.addSample(i, j, Vec3(1, 1, 1));
          film.addSample(i, j, Vec3(1, 1, 1));
        }
      tracker.sample(film, 16, 1.5);
      expect(tracker.series().size() == 1_u);
      const ErrorPoint& p = tracker.series().back();
      expect(p.spp == 2.0_d) << "16 samples over 8 pixels";
      expect(p.seconds == 1.5_d);
      expect(std::abs(p.error.mse - 0.25) < 1e-6) << "1.0 vs 0.5 in every channel";

      tracker.start();
      expect(tracker.series().empty()) << "a new render starts a new curve";
    };
  };
};
