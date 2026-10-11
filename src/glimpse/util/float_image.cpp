// PFM I/O. The header is text ("PF" or "Pf", then width height, then a scale whose sign gives the byte order); the
// pixels are raw float32, one scanline after another, starting with the BOTTOM row of the image.

#include "glimpse/util/float_image.h"

#include <bit>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>

namespace glimpse {

namespace {

constexpr bool kHostLittleEndian = std::endian::native == std::endian::little;

float byteSwapped(float f) {
  uint32_t u;
  std::memcpy(&u, &f, 4);
  u = (u >> 24) | ((u >> 8) & 0x0000ff00u) | ((u << 8) & 0x00ff0000u) | (u << 24);
  std::memcpy(&f, &u, 4);
  return f;
}

}  // namespace

bool writePFM(const std::string& path, const FloatImage& image) {
  std::ofstream out(path, std::ios::binary);
  if (!out) return false;
  // The scale's sign tells readers the byte order of the floats that follow; we write in host order.
  out << "PF\n" << image.width << " " << image.height << "\n" << (kHostLittleEndian ? "-1" : "1") << "\n";
  const auto rowBytes = static_cast<std::streamsize>(image.width) * 3 * sizeof(float);
  for (int y = image.height - 1; y >= 0; --y)  // PFM: bottom row first
    out.write(reinterpret_cast<const char*>(&image.rgb[image.index(0, y)]), rowBytes);
  return static_cast<bool>(out);
}

std::optional<FloatImage> readPFM(const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) return std::nullopt;

  std::string magic;
  int width = 0, height = 0;
  float scale = 0.0f;
  in >> magic >> width >> height >> scale;
  if (!in || (magic != "PF" && magic != "Pf") || width <= 0 || height <= 0 || scale == 0.0f) return std::nullopt;
  in.get();  // the single whitespace character that ends the header

  const int channels = magic == "PF" ? 3 : 1;
  const bool swap = (scale < 0.0f) != kHostLittleEndian;  // file order differs from host order
  const float factor = std::fabs(scale);

  FloatImage image(width, height);
  std::vector<float> row(static_cast<size_t>(width) * channels);
  for (int y = height - 1; y >= 0; --y) {  // the file's first scanline is the image's bottom row
    in.read(reinterpret_cast<char*>(row.data()), static_cast<std::streamsize>(row.size() * sizeof(float)));
    if (!in) return std::nullopt;  // truncated
    for (int x = 0; x < width; ++x)
      for (int c = 0; c < 3; ++c) {
        float v = row[static_cast<size_t>(x) * channels + (channels == 3 ? c : 0)];
        if (swap) v = byteSwapped(v);
        image.rgb[image.index(x, y) + c] = v * factor;
      }
  }
  return image;
}

}  // namespace glimpse
