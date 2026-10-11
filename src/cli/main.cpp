// Glimpse_cli: render one scene to an image file.
// Flags are parsed in util/cli_options; the scene comes from scenes/registry; logs go to logs/cli_<time>.log.

#include <chrono>
#include <filesystem>
#include <iostream>

#include "glimpse/util/cli_options.h"
#include "glimpse/util/color.h"
#include "glimpse/util/float_image.h"
#include "glimpse/glimpse.h"
#include "glimpse/util/log.h"
#include "glimpse/render.h"

int main(int argc, char **argv) {
  using namespace glimpse;

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

  const std::string out = options.out.empty() ? defaultOutputPath(*sceneName) : options.out;
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
