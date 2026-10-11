#include <chrono>
#include <filesystem>
#include <iostream>

#include "glimpse/util/cli_options.h"
#include "glimpse/glimpse.h"
#include "glimpse/util/logger.h"
#include "glimpse/render.h"

using namespace glimpse;

int main(int argc, char **argv) {
  const ParseResult parsed = parse_command_line(argc, argv);
  if (!parsed.ok()) {
    std::cerr << "error: " << parsed.error << "\n\n" << usage();
    return 2;
  }
  const CmdOptions &options = parsed.options;

  if (options.help) {
    std::cout << usage();
    return 0;
  }
  if (options.list_scenes) {
    for (size_t i = 0; i < Scene::SceneNames.size(); ++i) std::cout << i << "  " << Scene::SceneNames[i] << "\n";
    return 0;
  }

  const auto scene_name = resolve_scene_name(options.scene);
  if (!scene_name) {
    std::cerr << "error: unknown scene '" << options.scene << "' (see --list-scenes)\n";
    return 2;
  }

  Logger logger(log_file_for("cli"));
  // Seeds every worker thread's generator. Rows are split per thread, so this is reproducible on one machine;
  // seeding that is independent of the thread count is technique 03.
  if (options.seed) Random::set_seed(*options.seed);

  // Scenes declare their own lights (scene.lights); nothing is added here.
  Scene scene = Scene::SceneMap[*scene_name]();
  apply_options(options, scene);
  const camera &cam = scene.cam;
  Image image(cam.image_width, cam.image_height);

  // Stratified sampling uses a sqrt_spp x sqrt_spp grid: the real sample count is sqrt_spp^2.
  logger.log("Rendering ", *scene_name, " at ", cam.image_width, "x", cam.image_height, ", ", cam.sqrt_spp * cam.sqrt_spp,
             " spp, depth ", cam.max_depth);
  const auto start = std::chrono::high_resolution_clock::now();

  Renderer renderer;
  renderer.render_scene(scene, image, nullptr);

  const auto seconds = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - start).count();
  logger.log("Rendered in ", seconds, " s");

  const std::string out = options.out.empty() ? default_output_path(*scene_name) : options.out;
  const auto parent = std::filesystem::path(out).parent_path();
  if (!parent.empty()) std::filesystem::create_directories(parent);
  if (!image.write(out)) {
    logger.log("Failed to write ", out);
    return 1;
  }
  logger.log("Wrote ", out);
  return 0;
}
