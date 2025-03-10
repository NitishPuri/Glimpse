#pragma once

#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "vec3.h"

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

  // Add constructor from float values
  // ImageColor(float r, float g, float b) {
  ImageColor(float r, float g, float b) {
    rgb[0] = float_to_byte(r);
    rgb[1] = float_to_byte(g);
    rgb[2] = float_to_byte(b);
  }

 private:
  // Helper function to convert float to byte
  static uchar float_to_byte(float value) {
    // value = linear_to_gamma(value);
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
  void clear() { std::fill(pixel_data.begin(), pixel_data.end(), uchar{0}); }

  // Whether the image has valid data
  bool is_valid() const { return !pixel_data.empty(); }

  // Get/set pixel methods (byte-based)
  ImageColor get(int x, int y) const;
  void set(int x, int y, const ImageColor& color);

  // Get pixel as float values [0.0, 1.0]
  vec3 get_float(int x, int y) const;

  // Set pixel from float values [0.0, 1.0]
  void set_float(int x, int y, float r, float g, float b);

  // File I/O
  bool read(const std::string& filename);
  bool write(const std::string& filename) const;

  // Access raw data
  std::vector<uchar>& data() { return pixel_data; }
  const std::vector<uchar>& data() const { return pixel_data; }

 private:
  // Convert linear RGB [0,1] to sRGB [0,1]
  static float linear_to_srgb(float linear) {
    if (linear <= 0.0031308f) {
      return linear * 12.92f;
    } else {
      return 1.055f * pow(linear, 1.0f / 2.4f) - 0.055f;
    }
  }

  // Convert sRGB [0,1] to linear RGB [0,1]
  static float srgb_to_linear(float srgb) {
    if (srgb <= 0.04045f) {
      return srgb / 12.92f;
    } else {
      return pow((srgb + 0.055f) / 1.055f, 2.4f);
    }
  }
  std::vector<uchar> pixel_data;
  const int bytes_per_pixel = 3;  // RGB format

  // Helper methods
  int get_index(int x, int y) const;
  void clamp_coordinates(int& x, int& y) const;

  // For loading with STB
  std::unique_ptr<StbImageData> load_stb_image(const std::string& filename);
};

}  // namespace glimpse