// Flag parsing with full validation (every number must parse completely and be in range; only writable image
// formats are accepted).

#include "glimpse/util/cli_options.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <limits>
#include <sstream>
#include <string_view>

#include "glimpse/scenes/registry.h"

namespace glimpse {

namespace {

// The whole string must be a base-10 integer in [min, max]: rejects "", "12abc", out-of-range and overflow.
template <class T>
std::optional<T> parseInt(std::string_view text, long long min, long long max) {
  long long value = 0;
  const char* end = text.data() + text.size();
  auto [ptr, ec] = std::from_chars(text.data(), end, value);
  if (ec != std::errc{} || ptr != end || value < min || value > max) return std::nullopt;
  return static_cast<T>(value);
}

// Lower-case extension without the dot ("" if none).
std::string extensionOf(std::string_view path) {
  auto dot = path.find_last_of('.');
  auto slash = path.find_last_of('/');
  if (dot == std::string_view::npos || (slash != std::string_view::npos && dot < slash)) return "";
  std::string ext(path.substr(dot + 1));
  std::transform(ext.begin(), ext.end(), ext.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return ext;
}

// 8-bit formats go through Image::write (sRGB-encoded); .pfm is the linear float film (technique 01).
bool isWritableExtension(std::string_view path) {
  const std::string ext = extensionOf(path);
  return ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp" || ext == "tga" || ext == "pfm";
}

}  // namespace

ParseResult parseCommandLine(int argc, const char* const argv[]) {
  ParseResult result;
  CmdOptions& o = result.options;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--help" || arg == "-h") {
      o.help = true;
      continue;
    }
    if (arg == "--list-scenes") {
      o.listScenes = true;
      continue;
    }
    if (arg != "--scene" && arg != "--spp" && arg != "--width" && arg != "--depth" && arg != "--seed" &&
        arg != "--out" && arg != "--exposure") {
      result.error = "unknown argument: " + arg;
      return result;
    }
    if (i + 1 >= argc) {
      result.error = "missing value for " + arg;
      return result;
    }
    const std::string value = argv[++i];
    const std::string invalid = "invalid value for " + arg + ": '" + value + "'";

    if (arg == "--scene") {
      o.scene = value;
    } else if (arg == "--out") {
      if (!isWritableExtension(value)) {
        result.error = invalid + " (supported: .png .jpg .jpeg .bmp .tga .pfm)";
        return result;
      }
      o.out = value;
    } else if (arg == "--exposure") {
      double stops = 0.0;
      const char* end = value.data() + value.size();
      auto [ptr, ec] = std::from_chars(value.data(), end, stops);
      if (ec != std::errc{} || ptr != end || stops < -20.0 || stops > 20.0) {
        result.error = invalid + " (stops, -20 .. 20)";
        return result;
      }
      o.exposure = stops;
    } else if (arg == "--seed") {
      o.seed = parseInt<uint32_t>(value, 1, std::numeric_limits<uint32_t>::max());
      if (!o.seed) {
        result.error = invalid + " (1 .. 4294967295)";
        return result;
      }
    } else {
      // Generous upper bounds: they only catch typos, not taste.
      const long long max = arg == "--width" ? 16384 : arg == "--spp" ? 1'000'000 : 10'000;
      auto v = parseInt<int>(value, 1, max);
      if (!v) {
        result.error = invalid + " (1 .. " + std::to_string(max) + ")";
        return result;
      }
      (arg == "--spp" ? o.spp : arg == "--width" ? o.width : o.depth) = v;
    }
  }
  return result;
}

std::string usage() {
  std::ostringstream ss;
  ss << "usage: Glimpse_cli [--scene <name|index>] [--spp N] [--width N] [--depth N] [--seed N] [--out file]\n"
        "                   [--exposure STOPS] [--list-scenes] [--help]\n"
        "  --scene   scene name or index (default cornell_box; see --list-scenes)\n"
        "  --spp     samples per pixel; rounded down to a square (10 -> 9) for stratified sampling\n"
        "  --width   image width; height follows the scene's aspect ratio\n"
        "  --depth   maximum bounces\n"
        "  --seed    fixed RNG seed (>= 1); reproducible on the same machine and thread count\n"
        "  --out     output image: .png .jpg .bmp .tga (sRGB, 8-bit) or .pfm (linear float radiance)\n"
        "            (default results/<scene>.png)\n"
        "  --exposure  scale radiance by 2^STOPS before sRGB encoding (8-bit outputs only; default 0)\n";
  return ss.str();
}

std::optional<std::string> resolveSceneName(const std::string& nameOrIndex) {
  if (findScene(nameOrIndex)) return nameOrIndex;
  auto index = parseInt<size_t>(nameOrIndex, 0, static_cast<long long>(sceneRegistry().size()) - 1);
  if (index) return sceneRegistry()[*index].name;
  return std::nullopt;
}

void applyOptions(const CmdOptions& options, Scene& scene) {
  if (options.spp) scene.cam.samplesPerPixel = *options.spp;
  if (options.width) scene.cam.imageWidth = *options.width;
  if (options.depth) scene.cam.maxDepth = *options.depth;
  scene.cam.initialize();
}

std::string defaultOutputPath(const std::string& sceneName) { return "results/" + sceneName + ".png"; }

bool isFloatOutput(const std::string& path) { return extensionOf(path) == "pfm"; }

}  // namespace glimpse
