#pragma once

#include "core/color.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <string>

#include "stb/stb_image_write.h"

class Image {
 public:
  void initialize(int w, int h) {
    data.clear();

    width = w;
    height = h;
    data = std::vector<uint8_t>(width * height * 3);
  }

  Image() {
    width = 0;
    height = 0;
  }

  Image(int w, int h) : width(w), height(h) { initialize(w, h); }

  void clear() { memset(data.data(), 0, data.size()); }

  void Image::set(int x, int y, ImageColor color) {
    if (x < 0 || y < 0 || x >= width || y >= height) {
      return;
    }

    // data[(x + y * width) * num_components + 0] = color.rgb[0];
    // data[(x + y * width) * num_components + 1] = color.rgb[1];
    // data[(x + y * width) * num_components + 2] = color.rgb[2];

    memcpy(data.data() + (x + y * width) * num_components, color.rgb,
           num_components);
  }

  // Color get(int x, int y) const {
  //     Color c {};
  //     c.rgb[0] = data[x + y * width + 0];
  //     c.rgb[1] = data[x + y * width + 1];
  //     c.rgb[2] = data[x + y * width + 2];
  //     return c;
  // }

  int write(const std::string &filename, bool flip = true) {
    stbi_flip_vertically_on_write(1);
    return stbi_write_jpg(filename.c_str(), width, height, num_components,
                          data.data(), 95);
    // return stbi_write_bmp(filename.c_str(), width, height, num_components,
    // data);
  }

  //  private:
  std::vector<uint8_t> data;
  int width = 0;
  int height = 0;
  const int num_components = 3;
};
