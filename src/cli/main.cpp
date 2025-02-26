
#include <future>

#include "core/cli.h"
#include "core/glimpse.h"
#include "core/logger.h"
#include "core/render.h"

const std::string log_file_path = "./log_cli.txt";

int main(int argc, char **argv) {
  Logger logger(log_file_path);
  logger.log("Starting...");

  // TODO: Parameterize scene from cli
  auto options = ParseCommandLine(argc, argv);
  // logger.log("Width: ", options.width);
  // logger.log("Height: ", options.height);
  logger.log("Samples: ", options.samples);
  logger.log("Max Depth: ", options.max_depth);
  logger.log("Scene: ", options.scene);

  // Scene
  auto scene = Scene::SceneMap[Scene::SceneNames[options.scene]]();

  // Image
  auto aspect_ratio = scene.cam.aspect_ratio;
  int image_width = scene.cam.image_width;
  int samples_per_pixel = scene.cam.samples_per_pixel;
  int max_depth = scene.cam.max_depth;

  const int image_height = static_cast<int>(image_width / aspect_ratio);
  Image image(image_width, image_height);

  // Render
  logger.log("Rendering... ", image_width, "x", image_height, " with ",
             samples_per_pixel, " samples per pixel");
  auto startTime = std::chrono::high_resolution_clock::now();

  Renderer::render_scene(scene, image);

  auto endTime = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration<float, std::chrono::seconds::period>(
                      endTime - startTime)
                      .count();

  logger.log("Image generated in ", duration, " seconds");

  std::string result_path("./results/final21.jpg");
  logger.log("Writing result to :: ", result_path);
  if (image.write(result_path) != 0) {
    logger.log("Image written to ", result_path);
  } else {
    logger.log("Failed to write image to ", result_path);
  }
}