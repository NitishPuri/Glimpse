#pragma once

// ErrorTracker: the viewer's live error against the scene's reference, refs/<scene>.pfm (technique 02).
// A reference is a converged render of the scene as its factory builds it (only the width may differ, via the CLI's
// --width), so the error means something only while the viewer renders that same image: same size, camera, depth
// and background. After any edit the tracker reports why it can't measure instead of plotting the error of a
// different image.

#include <cstdint>
#include <format>
#include <optional>
#include <string>
#include <vector>

#include "glimpse/film.h"
#include "glimpse/scene.h"
#include "glimpse/util/cli_options.h"
#include "glimpse/util/float_image.h"
#include "glimpse/util/metrics.h"

namespace glimpse::gui {

struct ErrorPoint {
  double spp = 0;      // average samples per pixel so far
  double seconds = 0;  // since the render started
  ErrorStats error;
};

class ErrorTracker {
 public:
  // On scene setup: loads the scene's reference, if one has been rendered, and remembers the scene state it is for.
  void loadReference(const std::string& sceneName, const Scene& scene) {
    path = referenceOutputPath(sceneName);
    reference = readPFM(path);
    referenceCamera = scene.cam;
    referenceBackground = scene.background;
    points.clear();
  }

  const std::optional<FloatImage>& ref() const { return reference; }

  // Empty if `scene` renders the reference's image; otherwise why not, for the UI.
  std::string unavailableReason(const Scene& scene) const {
    if (!reference) return "no reference (Glimpse_cli --scene <name> --reference writes " + path + ")";
    const Camera& c = scene.cam;
    const Camera& r = referenceCamera;
    // imageHeight is only derived in Camera::initialize, which runs on the render thread: derive it here too.
    if (c.imageWidth != reference->width || static_cast<int>(c.imageWidth / c.aspectRatio) != reference->height)
      return std::format("render size differs from the reference's {}x{}", reference->width, reference->height);
    if (!(c.lookfrom == r.lookfrom) || !(c.lookat == r.lookat) || !(c.vup == r.vup) || c.vfov != r.vfov ||
        c.defocusAngle != r.defocusAngle || c.focusDistance != r.focusDistance)
      return "camera moved since the reference";
    if (c.maxDepth != r.maxDepth) return "max depth differs from the reference's";
    if (!(scene.background == referenceBackground)) return "background differs from the reference's";
    return "";
  }

  void start() { points.clear(); }

  // Measures the film's current estimate. Called from the UI thread while the workers keep adding samples: a pixel
  // read mid-update can be off by one sample, which is noise next to the error being shown (the viewer's image
  // upload has the same race). Pixels not reached yet read as black and count as error until they are.
  void sample(const Film& film, int64_t samplesTraced, double seconds) {
    if (!reference) return;
    const auto stats = compareImages(film.linearRGB(), *reference);
    if (!stats) return;
    const double pixels = static_cast<double>(reference->width) * reference->height;
    points.push_back({static_cast<double>(samplesTraced) / pixels, seconds, *stats});
  }

  const std::vector<ErrorPoint>& series() const { return points; }

 private:
  std::string path;
  std::optional<FloatImage> reference;
  Camera referenceCamera;
  Color referenceBackground;
  std::vector<ErrorPoint> points;
};

}  // namespace glimpse::gui
