#include "glimpse/util/cli_options.h"

#include <initializer_list>
#include <vector>

#include "glimpse/scenes.h"

//
#include "../test_cfg.h"

using namespace glimpse;

namespace {
ParseResult parse(std::initializer_list<const char*> args) {
  std::vector<const char*> argv{"Glimpse_cli"};
  argv.insert(argv.end(), args.begin(), args.end());
  return parse_command_line(static_cast<int>(argv.size()), argv.data());
}
}  // namespace

void cli_test() {
  using namespace boost::ut;

  "cli"_test = [] {
    "defaults"_test = [] {
      auto r = parse({});
      expect(r.ok()) << r.error;
      expect(r.options.scene == "cornell_box");
      expect(!r.options.spp.has_value() && !r.options.width.has_value() && !r.options.depth.has_value());
      expect(!r.options.seed.has_value());
      expect(r.options.out.empty());
      expect(!r.options.help && !r.options.list_scenes);
    };

    "all_flags"_test = [] {
      auto r = parse({"--scene", "earth", "--spp", "16", "--width", "320", "--depth", "8", "--seed", "42", "--out",
                      "renders/e.png"});
      expect(r.ok()) << r.error;
      expect(r.options.scene == "earth");
      expect(r.options.spp.value_or(-1) == 16_i);
      expect(r.options.width.value_or(-1) == 320_i);
      expect(r.options.depth.value_or(-1) == 8_i);
      expect(r.options.seed.value_or(0u) == 42_u);
      expect(r.options.out == "renders/e.png");
    };

    "help_and_list"_test = [] {
      expect(parse({"--help"}).options.help);
      expect(parse({"-h"}).options.help);
      expect(parse({"--list-scenes"}).options.list_scenes);
      expect(usage().find("--spp") != std::string::npos);
    };

    "rejects_bad_input"_test = [] {
      expect(!parse({"--spp"}).ok()) << "missing value";
      expect(!parse({"--spp", "abc"}).ok()) << "not a number";
      expect(!parse({"--spp", "12abc"}).ok()) << "trailing garbage";
      expect(!parse({"--spp", "0"}).ok()) << "zero samples";
      expect(!parse({"--width", "-5"}).ok()) << "negative width";
      expect(!parse({"--width", "100000"}).ok()) << "absurd width";
      expect(!parse({"--seed", "0"}).ok()) << "0 means 'random' in Random::set_seed";
      expect(!parse({"--bogus"}).ok()) << "unknown flag";
      expect(parse({"--bogus"}).error.find("--bogus") != std::string::npos) << "error names the flag";
    };

    "out_extension_validation"_test = [] {
      expect(parse({"--out", "a.png"}).ok());
      expect(parse({"--out", "a.JPG"}).ok());
      expect(parse({"--out", "a.jpeg"}).ok());
      expect(parse({"--out", "dir/a.bmp"}).ok());
      expect(parse({"--out", "a.tga"}).ok());
      expect(!parse({"--out", "a.pfm"}).ok()) << "PFM arrives in technique 01";
      expect(!parse({"--out", "a.exr"}).ok());
      expect(!parse({"--out", "noext"}).ok());
    };

    "resolve_scene"_test = [] {
      expect(resolve_scene_name("cornell_box").value_or("") == "cornell_box");
      expect(resolve_scene_name("9").value_or("") == Scene::SceneNames[9]);
      expect(resolve_scene_name("0").value_or("") == Scene::SceneNames[0]);
      expect(!resolve_scene_name("-1").has_value());
      expect(!resolve_scene_name(std::to_string(Scene::SceneNames.size())).has_value());
      expect(!resolve_scene_name("no_such_scene").has_value());
      expect(!resolve_scene_name("").has_value());
    };

    "apply_keeps_scene_values"_test = [] {
      Scene s = Scene::SceneMap["cornell_box"]();  // 600 px, 100 spp, aspect 1.0
      apply_options(CmdOptions{}, s);
      expect(s.cam.image_width == 600_i);
      expect(s.cam.image_height == 600_i) << "initialize() ran";
      expect(s.cam.samples_per_pixel == 100_i);
      expect(s.cam.sqrt_spp == 10_i);
    };

    "apply_spp_rounds_down"_test = [] {
      Scene s = Scene::SceneMap["cornell_box"]();
      CmdOptions o;
      o.spp = 10;
      apply_options(o, s);
      expect(s.cam.samples_per_pixel == 10_i);
      expect(s.cam.sqrt_spp == 3_i) << "a 3x3 stratified grid: 9 samples actually rendered";
    };

    "apply_width_recomputes_height"_test = [] {
      Scene s = Scene::SceneMap["earth"]();
      CmdOptions o;
      o.width = 101;
      o.depth = 7;
      apply_options(o, s);
      expect(s.cam.image_width == 101_i);
      expect(s.cam.image_height == static_cast<int>(101 / s.cam.aspect_ratio));
      expect(s.cam.max_depth == 7_i);
    };

    "default_output_path"_test = [] { expect(default_output_path("earth") == "results/earth.png"); };
  };
}
