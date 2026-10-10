#include "cli_options.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <limits>
#include <sstream>
#include <string_view>

#include "scenes.h"

namespace glimpse {

namespace {

// The whole string must be a base-10 integer in [min, max]: rejects "", "12abc", out-of-range and overflow.
template <class T>
std::optional<T> parse_int(std::string_view text, long long min, long long max) {
  long long value = 0;
  const char* end = text.data() + text.size();
  auto [ptr, ec] = std::from_chars(text.data(), end, value);
  if (ec != std::errc{} || ptr != end || value < min || value > max) return std::nullopt;
  return static_cast<T>(value);
}

// Formats Image::write can produce today. PFM arrives with technique 01 (float_output).
bool is_writable_extension(std::string_view path) {
  auto dot = path.find_last_of('.');
  auto slash = path.find_last_of('/');
  if (dot == std::string_view::npos || (slash != std::string_view::npos && dot < slash)) return false;
  std::string ext(path.substr(dot + 1));
  std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp" || ext == "tga";
}

}  // namespace

ParseResult parse_command_line(int argc, const char* const argv[]) {
  ParseResult result;
  CmdOptions& o = result.options;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--help" || arg == "-h") {
      o.help = true;
      continue;
    }
    if (arg == "--list-scenes") {
      o.list_scenes = true;
      continue;
    }
    if (arg != "--scene" && arg != "--spp" && arg != "--width" && arg != "--depth" && arg != "--seed" &&
        arg != "--out") {
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
      if (!is_writable_extension(value)) {
        result.error = invalid + " (supported: .png .jpg .jpeg .bmp .tga)";
        return result;
      }
      o.out = value;
    } else if (arg == "--seed") {
      o.seed = parse_int<uint32_t>(value, 1, std::numeric_limits<uint32_t>::max());
      if (!o.seed) {
        result.error = invalid + " (1 .. 4294967295)";
        return result;
      }
    } else {
      // Generous upper bounds: they only catch typos, not taste.
      const long long max = arg == "--width" ? 16384 : arg == "--spp" ? 1'000'000 : 10'000;
      auto v = parse_int<int>(value, 1, max);
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
        "                   [--list-scenes] [--help]\n"
        "  --scene   scene name or index (default cornell_box; see --list-scenes)\n"
        "  --spp     samples per pixel; rounded down to a square (10 -> 9) for stratified sampling\n"
        "  --width   image width; height follows the scene's aspect ratio\n"
        "  --depth   maximum bounces\n"
        "  --seed    fixed RNG seed (>= 1); reproducible on the same machine and thread count\n"
        "  --out     output image: .png .jpg .bmp .tga (default results/<scene>.png)\n";
  return ss.str();
}

std::optional<std::string> resolve_scene_name(const std::string& name_or_index) {
  if (Scene::SceneMap.count(name_or_index)) return name_or_index;
  auto index = parse_int<size_t>(name_or_index, 0, static_cast<long long>(Scene::SceneNames.size()) - 1);
  if (index) return Scene::SceneNames[*index];
  return std::nullopt;
}

void apply_options(const CmdOptions& options, Scene& scene) {
  if (options.spp) scene.cam.samples_per_pixel = *options.spp;
  if (options.width) scene.cam.image_width = *options.width;
  if (options.depth) scene.cam.max_depth = *options.depth;
  scene.cam.initialize();
}

std::string default_output_path(const std::string& scene_name) { return "results/" + scene_name + ".png"; }

}  // namespace glimpse
