#pragma once

// Image: an 8-bit RGB pixel buffer (row 0 = top, like image files) with stb_image read/write by file extension.
// setFloat() clamps and quantises [0,1] floats. Float (PFM) output arrives with technique 01.

#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "glimpse/util/vecmath.h"

namespace glimpse {

// Forward declaration to avoid including stb headers in this header
struct StbImageData;

using uchar = unsigned char;

// Color representation for a pixel
class ImageColor {
 public:
  unsigned char rgb[3];

  ImageColor() : rgb{0, 0, 0} {}

  ImageColor(uchar r, uchar g, uchar b) : rgb{r, g, b} {}
  ImageColor(int r, int g, int b) : rgb{static_cast<uchar>(r), static_cast<uchar>(g), static_cast<uchar>(b)} {}

  // Float channels in [0, 1], clamped and quantised to 8 bits.
  ImageColor(float r, float g, float b) {
    rgb[0] = floatToByte(r);
    rgb[1] = floatToByte(g);
    rgb[2] = floatToByte(b);
  }

 private:
  // Helper function to convert float to byte
  static uchar floatToByte(float value) {
    if (value <= 0.0f) return 0;
    if (value >= 1.0f) return 255;
    return static_cast<uchar>(256.0f * value);
  }
};

// Unified image class that handles both loading and saving
class Image {
 public:
  int width = 0;
  int height = 0;

  // std::vector<unsigned char>& data() const;

  // Constructors
  Image() = default;
  Image(int width, int height);
  Image(const std::string& filename);
  ~Image();

  // Initialize a blank image
  void initialize(int w, int h);

  // clear to blank image
  void clear() { std::fill(pixelData.begin(), pixelData.end(), uchar{0}); }

  // Whether the image has valid data
  bool isValid() const { return !pixelData.empty(); }

  // Get/set pixel methods (byte-based)
  ImageColor get(int x, int y) const;
  void set(int x, int y, const ImageColor& c);

  // Get pixel as float values [0.0, 1.0]
  Vec3 getFloat(int x, int y) const;

  // Set pixel from float values [0.0, 1.0]
  void setFloat(int x, int y, float r, float g, float b);

  // File I/O
  bool read(const std::string& filename);
  bool write(const std::string& filename) const;

  // Access raw data
  std::vector<uchar>& data() { return pixelData; }
  const std::vector<uchar>& data() const { return pixelData; }

 private:
  // Convert linear RGB [0,1] to sRGB [0,1]
  static float linearToSrgb(float linear) {
    if (linear <= 0.0031308f) {
      return linear * 12.92f;
    } else {
      return 1.055f * pow(linear, 1.0f / 2.4f) - 0.055f;
    }
  }

  // Convert sRGB [0,1] to linear RGB [0,1]
  static float srgbToLinear(float srgb) {
    if (srgb <= 0.04045f) {
      return srgb / 12.92f;
    } else {
      return pow((srgb + 0.055f) / 1.055f, 2.4f);
    }
  }
  std::vector<uchar> pixelData;
  const int bytesPerPixel = 3;  // RGB format

  // Helper methods
  int getIndex(int x, int y) const;
  void clampCoordinates(int& x, int& y) const;

  // For loading with STB
  std::unique_ptr<StbImageData> loadStbImage(const std::string& filename);
};

}  // namespace glimpse