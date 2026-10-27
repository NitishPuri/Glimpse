// Glimpse_cli: render one scene to an image file, measure its `--convergence`, or `compare` two renders.
// Flags are parsed in util/cli_options; the scene comes from scenes/registry; logs go to logs/cli_<time>.log.

#include <chrono>
#include <filesystem>
#include <iostream>

#include "glimpse/convergence.h"
#include "glimpse/util/cli_options.h"
#include "glimpse/util/color.h"
#include "glimpse/util/float_image.h"
#include "glimpse/glimpse.h"
#include "glimpse/util/log.h"
#include "glimpse/util/image.h"
#include "glimpse/util/metrics.h"
#include "glimpse/render.h"

namespace {

// `compare <test.pfm> <ref.pfm>`: prints the error of a render against a reference (technique 02).
int runCompare(int argc, const char *const *argv) {
  using namespace glimpse;
  const CompareParseResult parsed = parseCompareCommand(argc, argv);
  if (!parsed.ok()) {
    std::cerr << "error: " << parsed.error << "\n\n" << usage();
    return 2;
  }
  const CompareOptions &o = parsed.options;

  const auto test = readPFM(o.test);
  const auto ref = readPFM(o.ref);
  if (!test || !ref) {
    std::cerr << "error: cannot read " << (!test ? o.test : o.ref) << "\n";
    return 1;
  }
  const auto stats = compareImages(*test, *ref);
  if (!stats) {
    std::cerr << "error: size mismatch: " << test->width << "x" << test->height << " vs " << ref->width << "x"
              << ref->height << "\n";
    return 1;
  }

  // Full precision: these numbers get pasted into technique notes and compared across runs.
  std::cout.precision(6);
  std::cout << "test    " << o.test << "\nref     " << o.ref << "\nsize    " << ref->width << "x" << ref->height
            << "\nMSE     " << stats->mse << "\nRMSE    " << stats->rmse << "\nrelMSE  " << stats->relMSE << "\n";
  if (stats->nonFinite > 0) std::cout << "warning: " << stats->nonFinite << " NaN/inf channel values skipped\n";

  if (!o.errorImage.empty()) {
    const auto parent = std::filesystem::path(o.errorImage).parent_path();
    if (!parent.empty()) std::filesystem::create_directories(parent);
    if (!falseColorError(*test, *ref).write(o.errorImage)) {
      std::cerr << "error: cannot write " << o.errorImage << "\n";
      return 1;
    }
    std::cout << "wrote   " << o.errorImage << "\n";
  }
  return 0;
}

// `--convergence out.csv`: error vs spp and time against refs/<scene>.pfm (technique 02).
int runConvergence(const glimpse::CmdOptions &options, const std::string &sceneName, glimpse::Logger &logger) {
  using namespace glimpse;
  const std::string refPath = referenceOutputPath(sceneName);
  const auto ref = readPFM(refPath);
  if (!ref) {
    std::cerr << "error: no reference " << refPath << " (render one with --scene " << sceneName << " --reference)\n";
    return 1;
  }

  Scene scene = findScene(sceneName)->make();
  // The reference fixes the image size; an explicit --width must agree with it.
  if (!options.width) scene.cam.imageWidth = ref->width;
  applyOptions(options, scene);
  if (scene.cam.imageWidth != ref->width || scene.cam.imageHeight != ref->height) {
    std::cerr << "error: render size " << scene.cam.imageWidth << "x" << scene.cam.imageHeight
              << " doesn't match the reference " << ref->width << "x" << ref->height << "\n";
    return 2;
  }

  const auto schedule = convergenceSchedule(options.spp.value_or(kConvergenceMaxSpp));
  logger.log("Convergence of ", sceneName, " at ", ref->width, "x", ref->height, ", depth ", scene.cam.maxDepth,
             ", ", schedule.size(), " renders up to ", schedule.back(), " spp, against ", refPath);
  const auto points = measureConvergence(scene, *ref, schedule, [&](const ConvergencePoint &p) {
    logger.log(p.spp, " spp  ", p.seconds, " s  RMSE ", p.error.rmse, "  relMSE ", p.error.relMSE);
  });
  if (points.size() != schedule.size()) {
    logger.log("Convergence stopped after ", points.size(), " of ", schedule.size(), " renders");
    return 1;
  }

  const auto parent = std::filesystem::path(options.convergence).parent_path();
  if (!parent.empty()) std::filesystem::create_directories(parent);
  if (!writeConvergenceCSV(options.convergence, points)) {
    logger.log("Failed to write ", options.convergence);
    return 1;
  }
  logger.log("Wrote ", options.convergence);
  return 0;
}

}  // namespace

int main(int argc, char **argv) {
  using namespace glimpse;

  if (isCompareCommand(argc, argv)) return runCompare(argc, argv);

  const ParseResult parsed = parseCommandLine(argc, argv);
  if (!parsed.ok()) {
    std::cerr << "error: " << parsed.error << "\n\n" << usage();
    return 2;
  }
  const CmdOptions &options = parsed.options;

  if (options.help) {
    std::cout << usage();
    return 0;
  }
  if (options.listScenes) {
    for (size_t i = 0; i < sceneRegistry().size(); ++i) std::cout << i << "  " << sceneRegistry()[i].name << "\n";
    return 0;
  }

  const auto sceneName = resolveSceneName(options.scene);
  if (!sceneName) {
    std::cerr << "error: unknown scene '" << options.scene << "' (see --list-scenes)\n";
    return 2;
  }

  Logger logger(logFileFor("cli"));
  // Seeds every worker thread's generator. Rows are split per thread, so this is reproducible on one machine;
  // seeding that is independent of the thread count is technique 03.
  if (options.seed) Random::setSeed(*options.seed);

  if (!options.convergence.empty()) return runConvergence(options, *sceneName, logger);

  // Scenes declare their own lights (scene.lights); nothing is added here.
  Scene scene = findScene(*sceneName)->make();
  applyOptions(options, scene);
  const Camera &cam = scene.cam;
  Image image(cam.imageWidth, cam.imageHeight);

  // Stratified sampling uses a sqrtSpp x sqrtSpp grid: the real sample count is sqrtSpp^2.
  logger.log("Rendering ", *sceneName, " at ", cam.imageWidth, "x", cam.imageHeight, ", ", cam.sqrtSpp * cam.sqrtSpp,
             " spp, depth ", cam.maxDepth);
  const auto start = std::chrono::high_resolution_clock::now();

  Renderer renderer;
  renderer.renderScene(scene, image, nullptr);

  const auto seconds = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - start).count();
  logger.log("Rendered in ", seconds, " s");

  const std::string out = !options.out.empty() ? options.out
                          : options.reference ? referenceOutputPath(*sceneName)
                                              : defaultOutputPath(*sceneName);
  const auto parent = std::filesystem::path(out).parent_path();
  if (!parent.empty()) std::filesystem::create_directories(parent);
  // The output comes from the linear film: .pfm writes it as-is (lossless radiance, the measurement), 8-bit formats
  // get exposure + sRGB. The live `image` above is only the progress view and is not written.
  const FloatImage linear = renderer.film.linearRGB();
  const bool ok = isFloatOutput(out) ? writePFM(out, linear) : encodeSRGB(linear, options.exposure).write(out);
  if (!ok) {
    logger.log("Failed to write ", out);
    return 1;
  }
  logger.log("Wrote ", out);
  return 0;
}
