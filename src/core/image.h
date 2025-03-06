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

    memcpy(data.data() + (x + y * width) * num_components, color.rgb, num_components);
  }

  ImageColor get(int x, int y) const {
    // TODO: Should we add asserts and togglable exceptions here instead of silently ignoring such errors?
    //  We need correctness of the renderer for image orrrectness
    //  and if we can maintain an those invariants through exceptions, we should do so
    // This applies for all the code base that does soft error checking and silently ignores errors
    if (x < 0 || y < 0 || x >= width || y >= height) {
      return ImageColor(0, 0, 0);
    }

    ImageColor color{0, 0, 0};
    memcpy(color.rgb, data.data() + (x + y * width) * num_components, num_components);
    return color;
  }

  int write(const std::string &filename, bool flip = true);

  std::vector<uint8_t> data;
  int width = 0;
  int height = 0;
  const int num_components = 3;
};
