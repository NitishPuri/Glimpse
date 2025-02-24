#pragma once

#include "color.h"
#include "common.h"

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

  void set(int x, int y, ImageColor color) {
    if (x < 0 || y < 0 || x >= width || y >= height) {
      return;
    }

    memcpy(data.data() + (x + y * width) * num_components, color.rgb,
           num_components);
  }

  int write(const std::string &filename, bool flip = true);

  std::vector<uint8_t> data;
  int width = 0;
  int height = 0;
  const int num_components = 3;
};
