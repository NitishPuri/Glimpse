#include "glimpse/util/cli_options.h"

#include <initializer_list>
#include <vector>

#include "glimpse/scenes/registry.h"

//
#include "../test_cfg.h"

using namespace glimpse;

namespace {
// The parser's message on failure, "" on success: for `expect(r.has_value()) << errorOf(r)`.
template <typename T>
std::string errorOf(const std::expected<T, std::string>& r) {
  return r ? "" : r.error();
}

std::expected<CmdOptions, std::string> parse(std::initializer_list<const char*> args) {
  std::vector<const char*> argv{"Glimpse_cli"};
  argv.insert(argv.end(), args.begin(), args.end());
  return parseCommandLine(static_cast<int>(argv.size()), argv.data());
}

std::expected<CompareOptions, std::string> parseCompare(std::initializer_list<const char*> args) {
  std::vector<const char*> argv{"Glimpse_cli", "compare"};
  argv.insert(argv.end(), args.begin(), args.end());
  return parseCompareCommand(static_cast<int>(argv.size()), argv.data());
}
}  // namespace

static ut::suite<"cli"> cliSuite = [] {
  using namespace boost::ut;

  "cli"_test = [] {
    "defaults"_test = [] {
      auto r = parse({});
      expect(r.has_value() >> fatal) << errorOf(r);
      expect(r->scene == "cornell_box");
      expect(!r->spp.has_value() && !r->width.has_value() && !r->depth.has_value());
      expect(!r->seed.has_value());
      expect(r->out.empty());
      expect(!r->help && !r->listScenes);
    };

    "all_flags"_test = [] {
      auto r = parse({"--scene", "earth", "--spp", "16", "--width", "320", "--depth", "8", "--seed", "42", "--out",
                      "renders/e.png"});
      expect(r.has_value() >> fatal) << errorOf(r);
      expect(r->scene == "earth");
      expect(r->spp.value_or(-1) == 16_i);
      expect(r->width.value_or(-1) == 320_i);
      expect(r->depth.value_or(-1) == 8_i);
      expect(r->seed.value_or(0u) == 42_u);
      expect(r->out == "renders/e.png");
    };

    "exposure"_test = [] {
      expect(parse({}).value().exposure == 0.0_d) << "default: no exposure change";
      expect(parse({"--exposure", "1.5"}).value().exposure == 1.5_d);
      expect(parse({"--exposure", "-2"}).value().exposure == -2.0_d);
      expect(!parse({"--exposure", "abc"}).has_value());
      expect(!parse({"--exposure", "1.5x"}).has_value()) << "trailing garbage";
      expect(!parse({"--exposure", "100"}).has_value()) << "outside [-20, 20] stops";
      expect(!parse({"--exposure"}).has_value()) << "missing value";
    };

    // A reference is the converged image later techniques are measured against: lossless and high spp.
    "reference"_test = [] {
      auto r = parse({"--reference"});
      expect(r.has_value() >> fatal) << errorOf(r);
      expect(r->reference);
      expect(referenceOutputPath("cornell_box") == "refs/cornell_box.pfm");

      Scene s = findScene("cornell_box")->make();
      applyOptions(*r, s);
      expect(s.cam.samplesPerPixel == 4096_i) << "default reference spp";
      expect(s.cam.sqrtSpp == 64_i);

      auto r2 = parse({"--reference", "--spp", "256"});
      Scene s2 = findScene("cornell_box")->make();
      applyOptions(r2.value(), s2);
      expect(s2.cam.samplesPerPixel == 256_i) << "--spp still wins";

      expect(parse({"--reference", "--out", "x/ref.pfm"}).has_value());
      expect(!parse({"--reference", "--out", "x/ref.png"}).has_value()) << "a reference must be lossless (.pfm)";
      expect(!parse({"--out", "x/ref.png", "--reference"}).has_value()) << "order doesn't matter";
    };

    "help_and_list"_test = [] {
      expect(parse({"--help"}).value().help);
      expect(parse({"-h"}).value().help);
      expect(parse({"--list-scenes"}).value().listScenes);
      expect(usage().contains("--spp"));
    };

    "rejects_bad_input"_test = [] {
      expect(!parse({"--spp"}).has_value()) << "missing value";
      expect(!parse({"--spp", "abc"}).has_value()) << "not a number";
      expect(!parse({"--spp", "12abc"}).has_value()) << "trailing garbage";
      expect(!parse({"--spp", "0"}).has_value()) << "zero samples";
      expect(!parse({"--width", "-5"}).has_value()) << "negative width";
      expect(!parse({"--width", "100000"}).has_value()) << "absurd width";
      expect(!parse({"--seed", "0"}).has_value()) << "0 means 'random' in Random::setSeed";
      expect(!parse({"--bogus"}).has_value()) << "unknown flag";
      expect(parse({"--bogus"}).error().contains("--bogus")) << "error names the flag";
    };

    "out_extension_validation"_test = [] {
      expect(parse({"--out", "a.png"}).has_value());
      expect(parse({"--out", "a.JPG"}).has_value());
      expect(parse({"--out", "a.jpeg"}).has_value());
      expect(parse({"--out", "dir/a.bmp"}).has_value());
      expect(parse({"--out", "a.tga"}).has_value());
      expect(parse({"--out", "a.pfm"}).has_value()) << "linear float output (technique 01)";
      expect(parse({"--out", "refs/a.PFM"}).has_value());
      expect(!parse({"--out", "a.exr"}).has_value());
      expect(!parse({"--out", "noext"}).has_value());
    };

    "resolve_scene"_test = [] {
      expect(resolveSceneName("cornell_box").value_or("") == "cornell_box");
      expect(resolveSceneName("9").value_or("") == std::string(sceneRegistry()[9].name));
      expect(resolveSceneName("0").value_or("") == std::string(sceneRegistry()[0].name));
      expect(!resolveSceneName("-1").has_value());
      expect(!resolveSceneName(std::to_string(sceneRegistry().size())).has_value());
      expect(!resolveSceneName("no_such_scene").has_value());
      expect(!resolveSceneName("").has_value());
    };

    "apply_keeps_scene_values"_test = [] {
      Scene s = findScene("cornell_box")->make();  // 600 px, 100 spp, aspect 1.0
      applyOptions(CmdOptions{}, s);
      expect(s.cam.imageWidth == 600_i);
      expect(s.cam.imageHeight == 600_i) << "initialize() ran";
      expect(s.cam.samplesPerPixel == 100_i);
      expect(s.cam.sqrtSpp == 10_i);
    };

    "apply_spp_rounds_down"_test = [] {
      Scene s = findScene("cornell_box")->make();
      CmdOptions o;
      o.spp = 10;
      applyOptions(o, s);
      expect(s.cam.samplesPerPixel == 10_i);
      expect(s.cam.sqrtSpp == 3_i) << "a 3x3 stratified grid: 9 samples actually rendered";
    };

    "apply_width_recomputes_height"_test = [] {
      Scene s = findScene("earth")->make();
      CmdOptions o;
      o.width = 101;
      o.depth = 7;
      applyOptions(o, s);
      expect(s.cam.imageWidth == 101_i);
      expect(s.cam.imageHeight == static_cast<int>(101 / s.cam.aspectRatio));
      expect(s.cam.maxDepth == 7_i);
    };

    "default_output_path"_test = [] { expect(defaultOutputPath("earth") == "results/earth.png"); };

    "convergence"_test = [] {
      auto r = parse({"--convergence", "out/cb.csv", "--spp", "256"});
      expect(r.has_value() >> fatal) << errorOf(r);
      expect(r->convergence == "out/cb.csv");
      expect(parse({}).value().convergence.empty()) << "default: a normal render";
      expect(!parse({"--convergence", "cb.png"}).has_value()) << "CSV only";
      expect(!parse({"--convergence"}).has_value()) << "missing value";
      expect(!parse({"--convergence", "cb.csv", "--reference"}).has_value());
      expect(!parse({"--convergence", "cb.csv", "--out", "cb.pfm"}).has_value()) << "it writes no image";
    };
  };

  "cli_compare"_test = [] {
    "detected_only_as_first_argument"_test = [] {
      const char* cmp[] = {"Glimpse_cli", "compare", "a.pfm", "b.pfm"};
      const char* render[] = {"Glimpse_cli", "--scene", "compare"};
      expect(isCompareCommand(4, cmp));
      expect(!isCompareCommand(3, render)) << "a scene named 'compare' is not the subcommand";
      expect(!isCompareCommand(1, cmp));
    };

    "two_images"_test = [] {
      auto r = parseCompare({"test.pfm", "refs/ref.pfm"});
      expect(r.has_value() >> fatal) << errorOf(r);
      expect(r->test == "test.pfm");
      expect(r->ref == "refs/ref.pfm");
      expect(r->errorImage.empty());
    };

    "error_image"_test = [] {
      auto r = parseCompare({"a.pfm", "--error-image", "out/err.png", "b.pfm"});
      expect(r.has_value() >> fatal) << errorOf(r);
      expect(r->test == "a.pfm" && r->ref == "b.pfm") << "flag may sit between the images";
      expect(r->errorImage == "out/err.png");
      expect(!parseCompare({"a.pfm", "b.pfm", "--error-image", "err.pfm"}).has_value()) << "error image is 8-bit only";
      expect(!parseCompare({"a.pfm", "b.pfm", "--error-image"}).has_value()) << "missing value";
    };

    "rejects"_test = [] {
      expect(!parseCompare({}).has_value());
      expect(!parseCompare({"a.pfm"}).has_value()) << "one image";
      expect(!parseCompare({"a.pfm", "b.pfm", "c.pfm"}).has_value()) << "three images";
      expect(!parseCompare({"a.png", "b.pfm"}).has_value())
          << "8-bit sRGB would measure the tone curve, not the estimator";
      expect(!parseCompare({"a.pfm", "b.pfm", "--spp", "4"}).has_value()) << "unknown flag";
    };
  };
};
