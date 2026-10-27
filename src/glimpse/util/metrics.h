#pragma once

// Image error metrics against a reference: the numbers every technique is judged by (technique 02).
// MSE measures absolute error, so it is dominated by the brightest pixels. relMSE divides each pixel's squared error
// by its reference value squared, so a 10% error counts about the same in a dark corner as on a bright wall; it is
// the usual metric in rendering papers. Definitions match pbrt-v4's Image::MSE / Image::MRSE (`imgtool diff
// --metric MRSE` cross-checks them). The +0.01 keeps black reference pixels from dividing by zero.
// Follows pbrt-v4 §2.1–2.2 (estimator variance, efficiency) and RTG1 ch15.

#include <cstddef>
#include <optional>

namespace glimpse {

struct FloatImage;
class Image;

// Added to the reference value before squaring, as in pbrt's MRSE.
constexpr double kRelMSEEpsilon = 0.01;

struct ErrorStats {
  double mse = 0;     // mean over pixels and channels of (test - ref)^2
  double rmse = 0;    // sqrt(mse), in radiance units
  double relMSE = 0;  // mean of (test - ref)^2 / (ref + 0.01)^2 (pbrt's MRSE)
  size_t nonFinite = 0;  // NaN/inf channel values in either image: skipped, but counted so they can't hide
};

// nullopt if the images differ in size.
std::optional<ErrorStats> compareImages(const FloatImage& test, const FloatImage& ref);

// Per-pixel relative squared error (channel mean), shown on a log scale from `lo` (black) to `hi` (pale yellow)
// through purple, red and orange. Zero error is black; non-finite pixels are bright cyan.
Image falseColorError(const FloatImage& test, const FloatImage& ref, double lo = 1e-4, double hi = 1.0);

}  // namespace glimpse
