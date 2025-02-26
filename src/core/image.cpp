
#include "image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

int Image::write(const std::string &filename, bool flip) {
  stbi_flip_vertically_on_write(1);
  return stbi_write_jpg(filename.c_str(), width, height, num_components, data.data(), 95);
  // return stbi_write_bmp(filename.c_str(), width, height, num_components,
  // data);
}
