#include "image.h"

#include <algorithm>
#include <iostream>

// Define STB image implementation in this file
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG

// Include STB libraries
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

// Wrapper for STB image data
struct StbImageData {
  unsigned char* data = nullptr;
  int width = 0;
  int height = 0;
  int channels = 0;

  // Helper for float data
  float* float_data = nullptr;

  ~StbImageData() {
    if (data) stbi_image_free(data);
    if (float_data) stbi_image_free(float_data);
  }
};

// Constructors
Image::Image(int width, int height) { initialize(width, height); }

Image::Image(const std::string& filename) { read(filename); }

Image::~Image() = default;

void Image::initialize(int w, int h) {
  width = w;
  height = h;
  pixel_data.resize(width * height * bytes_per_pixel, 0);
}

// Pixel access methods
int Image::get_index(int x, int y) const { return (y * width + x) * bytes_per_pixel; }

void Image::clamp_coordinates(int& x, int& y) const {
  x = std::clamp(x, 0, width - 1);
  y = std::clamp(y, 0, height - 1);
}

ImageColor Image::get(int x, int y) const {
  if (!is_valid()) return ImageColor();

  clamp_coordinates(x, y);
  int idx = get_index(x, y);
  return ImageColor(pixel_data[idx], pixel_data[idx + 1], pixel_data[idx + 2]);
}

void Image::set(int x, int y, const ImageColor& color) {
  if (!is_valid()) return;

  clamp_coordinates(x, y);
  int idx = get_index(x, y);
  pixel_data[idx] = color.rgb[0];
  pixel_data[idx + 1] = color.rgb[1];
  pixel_data[idx + 2] = color.rgb[2];
}

// Float-based getters and setters
color Image::get_float(int x, int y) const {
  if (!is_valid()) {
    // r = 0.0f;
    // g = 0.0f;
    // b = 0.0f;
    return {};
  }

  clamp_coordinates(x, y);
  int idx = get_index(x, y);

  // Convert from byte [0, 255] to float [0.0, 1.0]
  return {pixel_data[idx] / 255.0f, pixel_data[idx + 1] / 255.0f, pixel_data[idx + 2] / 255.0f};
}

void Image::set_float(int x, int y, float r, float g, float b) {
  if (!is_valid()) return;

  // Clamp float values to [0.0, 1.0]
  r = std::clamp(r, 0.0f, 1.0f);
  g = std::clamp(g, 0.0f, 1.0f);
  b = std::clamp(b, 0.0f, 1.0f);

  // Convert to bytes and set
  set(x, y, ImageColor(r, g, b));
}

// Load image using STB library
std::unique_ptr<StbImageData> Image::load_stb_image(const std::string& filename) {
  auto result = std::make_unique<StbImageData>();

  // Try to load as 8-bit first
  result->data = stbi_load(filename.c_str(), &result->width, &result->height, &result->channels, bytes_per_pixel);

  if (!result->data) {
    std::cerr << "Failed to load image: " << stbi_failure_reason() << std::endl;
    return result;
  }

  // TODO: Using float data causes differences in read write loopback tests
  // Either remove float path or fix the issues.
  //  Also load float data for precision
  //  result->float_data =
  //      stbi_loadf(filename.c_str(), &result->width, &result->height, &result->channels, bytes_per_pixel);

  return result;
}

bool Image::read(const std::string& filename) {
  auto stb_data = load_stb_image(filename);

  if (!stb_data->data) {
    std::cerr << "Failed to load image: " << filename << std::endl;
    return false;
  }

  // Initialize our image with the loaded dimensions
  initialize(stb_data->width, stb_data->height);

  // Prefer float data for precision when available
  if (stb_data->float_data) {
    // Convert from float to our byte format
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        int stb_idx = (y * width + x) * bytes_per_pixel;
        float r = stb_data->float_data[stb_idx];
        float g = stb_data->float_data[stb_idx + 1];
        float b = stb_data->float_data[stb_idx + 2];
        set_float(x, y, r, g, b);
      }
    }
  } else {
    // Copy the byte data directly
    std::copy(stb_data->data, stb_data->data + pixel_data.size(), pixel_data.data());
  }

  return true;
}

// Write image using STB library
bool Image::write(const std::string& filename) const {
  if (!is_valid()) {
    std::cerr << "Cannot write invalid image to: " << filename << std::endl;
    return false;
  }

  // Determine file format from extension
  std::string ext;
  size_t dot_pos = filename.find_last_of('.');
  if (dot_pos != std::string::npos) {
    ext = filename.substr(dot_pos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
  }

  int result = 0;
  if (ext == "png") {
    result =
        stbi_write_png(filename.c_str(), width, height, bytes_per_pixel, pixel_data.data(), width * bytes_per_pixel);
  } else if (ext == "bmp") {
    result = stbi_write_bmp(filename.c_str(), width, height, bytes_per_pixel, pixel_data.data());
  } else if (ext == "jpg" || ext == "jpeg") {
    result = stbi_write_jpg(filename.c_str(), width, height, bytes_per_pixel, pixel_data.data(), 95);  // 95% quality
  } else if (ext == "tga") {
    result = stbi_write_tga(filename.c_str(), width, height, bytes_per_pixel, pixel_data.data());
  } else {
    // Default to PNG
    result =
        stbi_write_png(filename.c_str(), width, height, bytes_per_pixel, pixel_data.data(), width * bytes_per_pixel);
  }

  if (!result) {
    std::cerr << "Failed to write image: " << filename << std::endl;
    return false;
  }

  return true;
}