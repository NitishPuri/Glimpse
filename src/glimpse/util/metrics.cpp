#include "glimpse/util/metrics.h"

#include <algorithm>
#include <cmath>

#include "glimpse/util/float_image.h"
#include "glimpse/util/image.h"

namespace glimpse {

namespace {

double relativeSquaredError(double v, double ref) {
  const double d = v - ref;
  return d * d / ((ref + kRelMSEEpsilon) * (ref + kRelMSEEpsilon));
}

// Piecewise-linear colormap, dark to bright (in the spirit of matplotlib's "inferno"): t in [0, 1].
ImageColor colormap(double t) {
  struct Stop {
    double t;
    int r, g, b;
  };
  static constexpr Stop stops[] = {
      {0.00, 0, 0, 0}, {0.25, 60, 15, 110}, {0.50, 190, 40, 80}, {0.75, 250, 140, 20}, {1.00, 255, 255, 200}};
  t = std::clamp(t, 0.0, 1.0);
  for (size_t i = 1; i < std::size(stops); ++i) {
    if (t <= stops[i].t) {
      const Stop& a = stops[i - 1];
      const Stop& b = stops[i];
      const double f = (t - a.t) / (b.t - a.t);
      auto lerp = [f](int x, int y) { return static_cast<int>(std::lround(x + f * (y - x))); };
      return ImageColor(lerp(a.r, b.r), lerp(a.g, b.g), lerp(a.b, b.b));
    }
  }
  return ImageColor(255, 255, 200);
}

}  // namespace

std::optional<ErrorStats> compareImages(const FloatImage& test, const FloatImage& ref) {
  if (test.width != ref.width || test.height != ref.height) return std::nullopt;

  ErrorStats stats;
  double sumSE = 0, sumRSE = 0;
  for (size_t i = 0; i < ref.rgb.size(); ++i) {
    const double v = test.rgb[i], r = ref.rgb[i];
    if (!std::isfinite(v) || !std::isfinite(r)) {
      ++stats.nonFinite;
      continue;
    }
    sumSE += (v - r) * (v - r);
    sumRSE += relativeSquaredError(v, r);
  }
  // Like pbrt, divide by all channel values (skipped ones count as zero error), so a NaN can't shrink the mean.
  const double n = static_cast<double>(ref.rgb.size());
  stats.mse = n > 0 ? sumSE / n : 0;
  stats.rmse = std::sqrt(stats.mse);
  stats.relMSE = n > 0 ? sumRSE / n : 0;
  return stats;
}

Image falseColorError(const FloatImage& test, const FloatImage& ref, double lo, double hi) {
  Image out(ref.width, ref.height);
  if (test.width != ref.width || test.height != ref.height) return out;
  const double logLo = std::log10(lo), logHi = std::log10(hi);
  for (int y = 0; y < ref.height; ++y)
    for (int x = 0; x < ref.width; ++x) {
      const size_t i = ref.index(x, y);
      double rse = 0;
      bool finite = true;
      for (int c = 0; c < 3; ++c) {
        const double v = test.rgb[i + c], r = ref.rgb[i + c];
        finite = finite && std::isfinite(v) && std::isfinite(r);
        if (finite) rse += relativeSquaredError(v, r) / 3.0;
      }
      if (!finite) {
        out.set(x, y, ImageColor(0, 255, 255));  // impossible to miss
      } else {
        out.set(x, y, rse <= 0 ? colormap(0) : colormap((std::log10(rse) - logLo) / (logHi - logLo)));
      }
    }
  return out;
}

}  // namespace glimpse
