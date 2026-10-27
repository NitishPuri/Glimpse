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
  double exposure = 0.0;          // stops, applied before sRGB encoding (8-bit outputs only)
  std::string out;               // empty: defaultOutputPath(scene)
  bool reference = false;  // render a converged reference: linear .pfm, kReferenceSpp unless --spp is given
  bool listScenes = false;
  bool help = false;
};

// Samples per pixel for --reference (a 64x64 stratified grid): high enough that its noise is far below what the
// techniques under test leave, so error measured against it is the technique's error.
constexpr int kReferenceSpp = 4096;

struct ParseResult {
  CmdOptions options;
  std::string error;  // empty on success
  bool ok() const { return error.empty(); }
};

ParseResult parseCommandLine(int argc, const char* const argv[]);
std::string usage();

// `Glimpse_cli compare <test.pfm> <ref.pfm> [--error-image out.png]` (technique 02): error metrics of a render
// against a reference, the number every later technique is judged by.
struct CompareOptions {
  std::string test;
  std::string ref;
  std::string errorImage;  // empty: no false-colour image
};

struct CompareParseResult {
  CompareOptions options;
  std::string error;  // empty on success
  bool ok() const { return error.empty(); }
};

// True if argv[1] is the `compare` subcommand.
bool isCompareCommand(int argc, const char* const argv[]);

// Parses argv[2..]; argv[1] is "compare".
CompareParseResult parseCompareCommand(int argc, const char* const argv[]);

// Accepts a scene name or a decimal index into sceneRegistry().
std::optional<std::string> resolveSceneName(const std::string& nameOrIndex);

// Applies the overrides to the scene's camera, then recomputes its derived fields (imageHeight, sqrtSpp, ...).
void applyOptions(const CmdOptions& options, Scene& scene);

std::string defaultOutputPath(const std::string& sceneName);

// Where --reference writes by default: refs/<scene>.pfm (gitignored cache).
std::string referenceOutputPath(const std::string& sceneName);

// True for .pfm: the linear float film is written as-is (no exposure, no sRGB).
bool isFloatOutput(const std::string& path);

}  // namespace glimpse
