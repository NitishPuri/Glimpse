#pragma once

// Display encoding: linear radiance -> sRGB. The renderer computes linear values; screens and 8-bit files expect
// sRGB-encoded values, whose curve is roughly gamma 2.2 with a linear toe near black. This replaces RTIOW's
// sqrt ("gamma 2") approximation (technique 01; docs/diagrams/srgb_vs_sqrt.svg compares them).
// Follows pbrt-v4 §4.6 (color spaces, the sRGB transfer function).

namespace glimpse {

class Image;
struct FloatImage;

// sRGB transfer function (IEC 61966-2-1) for one channel; x is linear, typically in [0, 1].
double linearToSRGB(double x);

// Linear radiance -> 8-bit sRGB Image, after scaling by 2^exposureStops; values above 1 clamp to white.
Image encodeSRGB(const FloatImage& linear, double exposureStops);

}  // namespace glimpse
