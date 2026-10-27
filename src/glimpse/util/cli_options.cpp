// Flag parsing with full validation (every number must parse completely and be in range; only writable image
// formats are accepted).

#include "glimpse/util/cli_options.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <limits>
#include <sstream>
#include <string_view>
#include <vector>

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
    if (arg == "--reference") {
      o.reference = true;
      continue;
    }
    if (arg != "--scene" && arg != "--spp" && arg != "--width" && arg != "--depth" && arg != "--seed" &&
        arg != "--out" && arg != "--exposure" && arg != "--convergence") {
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
    } else if (arg == "--convergence") {
      if (extensionOf(value) != "csv") {
        result.error = invalid + " (a .csv file)";
        return result;
      }
      o.convergence = value;
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
  if (o.reference && !o.out.empty() && !isFloatOutput(o.out)) {
    result.error = "--reference writes linear radiance: --out must be a .pfm, not '" + o.out + "'";
  }
  // Convergence renders many images and keeps only their error, so an output image or a reference makes no sense.
  if (!o.convergence.empty() && (o.reference || !o.out.empty())) {
    result.error = "--convergence writes only its CSV: it can't be combined with --reference or --out";
  }
  return result;
}

std::string usage() {
  std::ostringstream ss;
  ss << "usage: Glimpse_cli [--scene <name|index>] [--spp N] [--width N] [--depth N] [--seed N] [--out file]\n"
        "                   [--exposure STOPS] [--reference] [--convergence file.csv]\n"
        "                   [--list-scenes] [--help]\n"
        "  --scene   scene name or index (default cornell_box; see --list-scenes)\n"
        "  --spp     samples per pixel; rounded down to a square (10 -> 9) for stratified sampling\n"
        "  --width   image width; height follows the scene's aspect ratio\n"
        "  --depth   maximum bounces\n"
        "  --seed    fixed RNG seed (>= 1); reproducible on the same machine and thread count\n"
        "  --out     output image: .png .jpg .bmp .tga (sRGB, 8-bit) or .pfm (linear float radiance)\n"
        "            (default results/<scene>.png)\n"
        "  --exposure  scale radiance by 2^STOPS before sRGB encoding (8-bit outputs only; default 0)\n"
        "  --reference render a converged reference: 4096 spp unless --spp, written to refs/<scene>.pfm\n"
        "  --convergence file.csv  render at 1, 4, 9, 16, 36, ... spp up to --spp (default 1024), writing\n"
        "            spp, seconds and error against refs/<scene>.pfm (image size taken from the reference)\n"
        "\n"
        "       Glimpse_cli compare <test.pfm> <ref.pfm> [--error-image out.png]\n"
        "  prints MSE, RMSE and relMSE of test against ref; --error-image writes per-pixel relative squared\n"
        "  error as a false-colour 8-bit image (log scale, black 1e-4 .. pale yellow 1; cyan = NaN/inf)\n";
  return ss.str();
}

bool isCompareCommand(int argc, const char* const argv[]) { return argc > 1 && std::string_view(argv[1]) == "compare"; }

CompareParseResult parseCompareCommand(int argc, const char* const argv[]) {
  CompareParseResult result;
  CompareOptions& o = result.options;
  std::vector<std::string> positional;

  for (int i = 2; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--error-image") {
      if (i + 1 >= argc) {
        result.error = "missing value for " + arg;
        return result;
      }
      o.errorImage = argv[++i];
      // The error image is a visualisation, so 8-bit only; the numbers are what gets measured.
      if (!isWritableExtension(o.errorImage) || isFloatOutput(o.errorImage)) {
        result.error = "invalid value for --error-image: '" + o.errorImage + "' (supported: .png .jpg .jpeg .bmp .tga)";
        return result;
      }
    } else if (arg.rfind("--", 0) == 0) {
      result.error = "unknown argument: " + arg;
      return result;
    } else {
      positional.push_back(arg);
    }
  }
  if (positional.size() != 2) {
    result.error = "compare takes two images, <test.pfm> <ref.pfm>";
    return result;
  }
  // Only float images: comparing 8-bit sRGB would measure the tone curve and quantisation, not the estimator.
  for (const std::string& path : positional) {
    if (!isFloatOutput(path)) {
      result.error = "compare needs linear .pfm images, not '" + path + "'";
      return result;
    }
  }
  o.test = positional[0];
  o.ref = positional[1];
  return result;
}

std::optional<std::string> resolveSceneName(const std::string& nameOrIndex) {
  if (findScene(nameOrIndex)) return nameOrIndex;
  auto index = parseInt<size_t>(nameOrIndex, 0, static_cast<long long>(sceneRegistry().size()) - 1);
  if (index) return sceneRegistry()[*index].name;
  return std::nullopt;
}

void applyOptions(const CmdOptions& options, Scene& scene) {
  if (options.spp) {
    scene.cam.samplesPerPixel = *options.spp;
  } else if (options.reference) {
    scene.cam.samplesPerPixel = kReferenceSpp;
  }
  if (options.width) scene.cam.imageWidth = *options.width;
  if (options.depth) scene.cam.maxDepth = *options.depth;
  scene.cam.initialize();
}

std::string defaultOutputPath(const std::string& sceneName) { return "results/" + sceneName + ".png"; }

std::string referenceOutputPath(const std::string& sceneName) { return "refs/" + sceneName + ".pfm"; }

bool isFloatOutput(const std::string& path) { return extensionOf(path) == "pfm"; }

}  // namespace glimpse
