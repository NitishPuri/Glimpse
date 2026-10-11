// Image I/O through stb_image / stb_image_write (compiled here), plus pixel access.

#include "glimpse/util/image.h"

#ifdef _MSC_VER
#pragma warning(push, 0)  // stb is compiled here; GCC/Clang get it via -isystem
#endif

#include <algorithm>
#include <iostream>

// Define STB image implementation in this file
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG

// Include STB libraries
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

namespace glimpse {
// Wrapper for STB image data
struct StbImageData {
  unsigned char* data = nullptr;
  int width = 0;
  int height = 0;
  int channels = 0;

  // Helper for float data
  float* floatData = nullptr;

  ~StbImageData() {
    if (data) stbi_image_free(data);
    if (floatData) stbi_image_free(floatData);
  }
};


// Constructors
Image::Image(int width, int height) { initialize(width, height); }

Image::Image(const std::string& filename) { read(filename); }

Image::~Image() = default;

void Image::initialize(int w, int h) {
  width = w;
  height = h;
  pixelData.resize(width * height * bytesPerPixel, 0);
}

// Pixel access methods
int Image::getIndex(int x, int y) const { return (y * width + x) * bytesPerPixel; }

void Image::clampCoordinates(int& x, int& y) const {
  x = std::clamp(x, 0, width - 1);
  y = std::clamp(y, 0, height - 1);
}

ImageColor Image::get(int x, int y) const {
  if (!isValid()) return ImageColor();

  clampCoordinates(x, y);
  int idx = getIndex(x, y);
  return ImageColor(pixelData[idx], pixelData[idx + 1], pixelData[idx + 2]);
}

void Image::set(int x, int y, const ImageColor& c) {
  if (!isValid()) return;

  clampCoordinates(x, y);
  int idx = getIndex(x, y);
  pixelData[idx] = c.rgb[0];
  pixelData[idx + 1] = c.rgb[1];
  pixelData[idx + 2] = c.rgb[2];
}

// Float-based getters and setters
Color Image::getFloat(int x, int y) const {
  if (!isValid()) return {};

  clampCoordinates(x, y);
  int idx = getIndex(x, y);

  // Convert from byte [0, 255] to float [0.0, 1.0]
  return {pixelData[idx] / 255.0f, pixelData[idx + 1] / 255.0f, pixelData[idx + 2] / 255.0f};
}

void Image::setFloat(int x, int y, float r, float g, float b) {
  if (!isValid()) return;

  // Clamp float values to [0.0, 1.0]
  r = std::clamp(r, 0.0f, 1.0f);
  g = std::clamp(g, 0.0f, 1.0f);
  b = std::clamp(b, 0.0f, 1.0f);

  // Convert to bytes and set
  set(x, y, ImageColor(r, g, b));
}

// Load image using STB library
std::unique_ptr<StbImageData> Image::loadStbImage(const std::string& filename) {
  auto result = std::make_unique<StbImageData>();

  // Try to load as 8-bit first
  result->data = stbi_load(filename.c_str(), &result->width, &result->height, &result->channels, bytesPerPixel);

  if (!result->data) {
    std::cerr << "Failed to load image: " << stbi_failure_reason() << std::endl;
    return result;
  }

  return result;
}

bool Image::read(const std::string& filename) {
  auto stbData = loadStbImage(filename);

  if (!stbData->data) {
    std::cerr << "Failed to load image: " << filename << std::endl;
    return false;
  }

  // Initialize our image with the loaded dimensions
  initialize(stbData->width, stbData->height);

  // Prefer float data for precision when available
  if (stbData->floatData) {
    // Convert from float to our byte format
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        int stbIdx = (y * width + x) * bytesPerPixel;
        float r = stbData->floatData[stbIdx];
        float g = stbData->floatData[stbIdx + 1];
        float b = stbData->floatData[stbIdx + 2];
        setFloat(x, y, r, g, b);
      }
    }
  } else {
    // Copy the byte data directly
    std::copy(stbData->data, stbData->data + pixelData.size(), pixelData.data());
  }

  return true;
}

// Write image using STB library
bool Image::write(const std::string& filename) const {
  if (!isValid()) {
    std::cerr << "Cannot write invalid image to: " << filename << std::endl;
    return false;
  }

  // Determine file format from extension
  std::string ext;
  size_t dotPos = filename.find_last_of('.');
  if (dotPos != std::string::npos) {
    ext = filename.substr(dotPos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  }

  int result = 0;
  if (ext == "png") {
    result =
        stbi_write_png(filename.c_str(), width, height, bytesPerPixel, pixelData.data(), width * bytesPerPixel);
  } else if (ext == "bmp") {
    result = stbi_write_bmp(filename.c_str(), width, height, bytesPerPixel, pixelData.data());
  } else if (ext == "jpg" || ext == "jpeg") {
    result = stbi_write_jpg(filename.c_str(), width, height, bytesPerPixel, pixelData.data(), 95);  // 95% quality
  } else if (ext == "tga") {
    result = stbi_write_tga(filename.c_str(), width, height, bytesPerPixel, pixelData.data());
  } else {
    // Default to PNG
    result =
        stbi_write_png(filename.c_str(), width, height, bytesPerPixel, pixelData.data(), width * bytesPerPixel);
  }

  if (!result) {
    std::cerr << "Failed to write image: " << filename << std::endl;
    return false;
  }

  return true;
}

}  // namespace glimpse

#ifdef _MSC_VER
#pragma warning(pop)
#endif
