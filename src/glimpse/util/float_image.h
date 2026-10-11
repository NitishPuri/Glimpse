#pragma once

// FloatImage: linear RGB floats, unclamped (radiance can exceed 1), row 0 = top like Image. PFM read/write.
// PFM ("portable float map") is the simplest lossless float format: a text header, then raw float32 scanlines,
// stored bottom-to-top. Technique 01 (float_output) writes Film radiance with it; reference renders in refs/ use it.
// Follows pbrt-v4 §B.5 (src/pbrt/util/image.cpp, ReadPFM/WritePFM).

#include <optional>
#include <string>
#include <vector>

namespace glimpse {

struct FloatImage {
  int width = 0;
  int height = 0;
  std::vector<float> rgb;  // 3 floats per pixel, row-major, row 0 = top

  FloatImage() = default;
  FloatImage(int w, int h) : width(w), height(h), rgb(static_cast<size_t>(w) * h * 3, 0.0f) {}

  // Offset of pixel (x, y)'s red channel in rgb.
  size_t index(int x, int y) const { return (static_cast<size_t>(y) * width + x) * 3; }
};

// Writes "PF\n<w> <h>\n-1\n" (negative scale = little-endian, the host order) and the scanlines bottom-to-top.
bool writePFM(const std::string& path, const FloatImage& image);

// Reads RGB ("PF") or grayscale ("Pf", replicated into RGB) PFM in either byte order, applying |scale|.
// nullopt if the file is missing, malformed or truncated.
std::optional<FloatImage> readPFM(const std::string& path);

}  // namespace glimpse
