// Glimpse_cli: render one scene to an image file, measure its `--convergence`, or `compare` two renders.
// Flags are parsed in util/cli_options; the scene comes from scenes/registry; logs go to logs/cli_<time>.log.

#include <chrono>
#include <filesystem>
#include <print>
#include <ranges>

#include "glimpse/convergence.h"
#include "glimpse/glimpse.h"
#include "glimpse/render.h"
#include "glimpse/util/cli_options.h"
#include "glimpse/util/color.h"
#include "glimpse/util/float_image.h"
#include "glimpse/util/image.h"
#include "glimpse/util/log.h"
#include "glimpse/util/metrics.h"

namespace {

// `compare <test.pfm> <ref.pfm>`: prints the error of a render against a reference (technique 02).
int runCompare(int argc, const char *const *argv) {
  using namespace glimpse;
  const auto parsed = parseCompareCommand(argc, argv);
  if (!parsed) {
    std::print(stderr, "error: {}\n\n{}", parsed.error(), usage());
    return 2;
  }
  const CompareOptions &o = *parsed;

  const auto test = readPFM(o.test);
  const auto ref = readPFM(o.ref);
  if (!test || !ref) {
    std::println(stderr, "error: {}: {}", !test ? o.test : o.ref, !test ? test.error() : ref.error());
    return 1;
  }
  const auto stats = compareImages(*test, *ref);
  if (!stats) {
    std::println(stderr, "error: size mismatch: {}x{} vs {}x{}", test->width, test->height, ref->width, ref->height);
    return 1;
  }

  // Six significant digits: these numbers get pasted into technique notes and compared across runs.
  std::println("test    {}\nref     {}\nsize    {}x{}", o.test, o.ref, ref->width, ref->height);
  std::println("MSE     {:.6g}\nRMSE    {:.6g}\nrelMSE  {:.6g}", stats->mse, stats->rmse, stats->relMSE);
  if (stats->nonFinite > 0) std::println("warning: {} NaN/inf channel values skipped", stats->nonFinite);

  if (!o.errorImage.empty()) {
    const auto parent = std::filesystem::path(o.errorImage).parent_path();
    if (!parent.empty()) std::filesystem::create_directories(parent);
    if (!falseColorError(*test, *ref).write(o.errorImage)) {
      std::println(stderr, "error: cannot write {}", o.errorImage);
      return 1;
    }
    std::println("wrote   {}", o.errorImage);
  }
  return 0;
}

// `--convergence out.csv`: error vs spp and time against refs/<scene>.pfm (technique 02).
int runConvergence(const glimpse::CmdOptions &options, const std::string &sceneName) {
  using namespace glimpse;
  const std::string refPath = referenceOutputPath(sceneName);
  const auto ref = readPFM(refPath);
  if (!ref) {
    std::println(stderr, "error: no reference: {}: {} (render one with --scene {} --reference)", refPath, ref.error(),
                 sceneName);
    return 1;
  }

  Scene scene = findScene(sceneName)->make();
  // The reference fixes the image size; an explicit --width must agree with it.
  if (!options.width) scene.cam.imageWidth = ref->width;
  applyOptions(options, scene);
  if (scene.cam.imageWidth != ref->width || scene.cam.imageHeight != ref->height) {
    std::println(stderr, "error: render size {}x{} doesn't match the reference {}x{}", scene.cam.imageWidth,
                 scene.cam.imageHeight, ref->width, ref->height);
    return 2;
  }

  const auto schedule = convergenceSchedule(options.spp.value_or(kConvergenceMaxSpp));
  log::info("Convergence of {} at {}x{}, depth {}, {} renders up to {} spp, against {}", sceneName, ref->width,
            ref->height, scene.cam.maxDepth, schedule.size(), schedule.back(), refPath);
  const auto points = measureConvergence(scene, *ref, schedule, [&](const ConvergencePoint &p) {
    log::info("{} spp  {:.3f} s  RMSE {:.6g}  relMSE {:.6g}", p.spp, p.seconds, p.error.rmse, p.error.relMSE);
  });
  if (points.size() != schedule.size()) {
    log::error("Convergence stopped after {} of {} renders", points.size(), schedule.size());
    return 1;
  }

  const auto parent = std::filesystem::path(options.convergence).parent_path();
  if (!parent.empty()) std::filesystem::create_directories(parent);
  if (!writeConvergenceCSV(options.convergence, points)) {
    log::error("Failed to write {}", options.convergence);
    return 1;
  }
  log::info("Wrote {}", options.convergence);
  return 0;
}

}  // namespace

// NOLINTNEXTLINE(bugprone-exception-escape): exceptions are for fatal failures; ending the run is the point
int main(int argc, char **argv) {
  using namespace glimpse;

  if (isCompareCommand(argc, argv)) return runCompare(argc, argv);

  const auto parsed = parseCommandLine(argc, argv);
  if (!parsed) {
    std::print(stderr, "error: {}\n\n{}", parsed.error(), usage());
    return 2;
  }
  const CmdOptions &options = *parsed;

  if (options.help) {
    std::print("{}", usage());
    return 0;
  }
  if (options.listScenes) {
    for (const auto &[i, entry] : std::views::enumerate(sceneRegistry())) std::println("{}  {}", i, entry.name);
    return 0;
  }

  const auto sceneName = resolveSceneName(options.scene);
  if (!sceneName) {
    std::println(stderr, "error: unknown scene '{}' (see --list-scenes)", options.scene);
    return 2;
  }

  log::setFile(logFileFor("cli"));
  // Seeds every worker thread's generator. Rows are split per thread, so this is reproducible on one machine;
  // seeding that is independent of the thread count is technique 03.
  if (options.seed) Random::setSeed(*options.seed);

  if (!options.convergence.empty()) return runConvergence(options, *sceneName);

  // Scenes declare their own lights (scene.lights); nothing is added here.
  Scene scene = findScene(*sceneName)->make();
  applyOptions(options, scene);
  const Camera &cam = scene.cam;
  Image image(cam.imageWidth, cam.imageHeight);

  // Stratified sampling uses a sqrtSpp x sqrtSpp grid: the real sample count is sqrtSpp^2.
  log::info("Rendering {} at {}x{}, {} spp, depth {}", *sceneName, cam.imageWidth, cam.imageHeight,
            cam.sqrtSpp * cam.sqrtSpp, cam.maxDepth);
  const auto start = std::chrono::high_resolution_clock::now();

  Renderer renderer;
  renderer.renderScene(scene, image, nullptr);

  const auto seconds = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - start).count();
  log::info("Rendered in {:.3f} s", seconds);

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
    log::error("Failed to write {}", out);
    return 1;
  }
  log::info("Wrote {}", out);
  return 0;
}
