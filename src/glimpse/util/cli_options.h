#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace glimpse {

struct Scene;

// Command-line options. Unset optionals mean "keep the scene's own value".
struct CmdOptions {
  std::string scene = "cornell_box";  // a name from Scene::SceneMap, or an index into Scene::SceneNames
  std::optional<int> spp;
  std::optional<int> width;  // height follows from the scene camera's aspect ratio
  std::optional<int> depth;
  std::optional<uint32_t> seed;  // >= 1; 0 is reserved by Random::set_seed for "random"
  std::string out;               // empty: default_output_path(scene)
  bool list_scenes = false;
  bool help = false;
};

struct ParseResult {
  CmdOptions options;
  std::string error;  // empty on success
  bool ok() const { return error.empty(); }
};

ParseResult parse_command_line(int argc, const char* const argv[]);
std::string usage();

// Accepts a scene name or a decimal index into Scene::SceneNames.
std::optional<std::string> resolve_scene_name(const std::string& name_or_index);

// Applies the overrides to the scene's camera, then recomputes its derived fields (image_height, sqrt_spp, ...).
void apply_options(const CmdOptions& options, Scene& scene);

std::string default_output_path(const std::string& scene_name);

}  // namespace glimpse
