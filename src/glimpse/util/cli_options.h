#pragma once

// Command-line options for Glimpse_cli: parse and validate flags, resolve a scene by name or index, apply the
// overrides to the scene's camera. Kept in the library so the unit tests cover it.

#include <cstdint>
#include <optional>
#include <string>

namespace glimpse {

struct Scene;

// Command-line options. Unset optionals mean "keep the scene's own value".
struct CmdOptions {
  std::string scene = "cornell_box";  // a scene name, or an index into sceneRegistry()
  std::optional<int> spp;
  std::optional<int> width;  // height follows from the scene camera's aspect ratio
  std::optional<int> depth;
  std::optional<uint32_t> seed;  // >= 1; 0 is reserved by Random::setSeed for "random"
  std::string out;               // empty: defaultOutputPath(scene)
  bool listScenes = false;
  bool help = false;
};

struct ParseResult {
  CmdOptions options;
  std::string error;  // empty on success
  bool ok() const { return error.empty(); }
};

ParseResult parseCommandLine(int argc, const char* const argv[]);
std::string usage();

// Accepts a scene name or a decimal index into sceneRegistry().
std::optional<std::string> resolveSceneName(const std::string& nameOrIndex);

// Applies the overrides to the scene's camera, then recomputes its derived fields (imageHeight, sqrtSpp, ...).
void applyOptions(const CmdOptions& options, Scene& scene);

std::string defaultOutputPath(const std::string& sceneName);

}  // namespace glimpse
