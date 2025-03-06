#ifndef RTW_STB_IMAGE_H
#define RTW_STB_IMAGE_H
//==============================================================================================
// To the extent possible under law, the author(s) have dedicated all copyright
// and related and neighboring rights to this software to the public domain
// worldwide. This software is distributed without any warranty.
//
// You should have received a copy (see file COPYING.txt) of the CC0 Public
// Domain Dedication along with this software. If not, see
// <http://creativecommons.org/publicdomain/zero/1.0/>.
//==============================================================================================

// Disable strict warnings for this header from the Microsoft Visual C++
// compiler.
#ifdef _MSC_VER
#pragma warning(push, 0)
#endif

#include <cstdlib>
#include <iostream>

// #include "stb/stb_image.h"

// TODO: Hook this with another envvariable <GLIMPSE_DATA> to look for stuff?
//  or maybe do that in another common place ( so that it works for models too)
class rtw_image {
 public:
  rtw_image() {}

  rtw_image(const char *image_filename) {
    auto filename = std::string(image_filename);
    // auto imagedir = getenv("RTW_IMAGES");

    // Hunt for the image file in some likely locations.
    // if (imagedir && load(std::string(imagedir) + "/" + image_filename))
    // return;
    if (load(filename)) return;

    std::cerr << "ERROR: Could not load image file '" << image_filename << "'.\n";
  }

  ~rtw_image();

  bool load(const std::string &filename);

  int width() const { return (fdata == nullptr) ? 0 : image_width; }
  int height() const { return (fdata == nullptr) ? 0 : image_height; }

  const unsigned char *pixel_data(int x, int y) const {
    // Return the address of the three RGB bytes of the pixel at x,y. If there
    // is no image data, returns magenta.
    static unsigned char magenta[] = {255, 0, 255};
    if (bdata == nullptr) return magenta;

    x = clamp(x, 0, image_width);
    y = clamp(y, 0, image_height);

    return bdata + y * bytes_per_scanline + x * bytes_per_pixel;
  }

  const float *pixel_data_f(int x, int y) const {
    // Return the address of the three RGB float values of the pixel at x,y.
    // If there is no image data, returns magenta.
    static float magenta[] = {1.0f, 0.0f, 1.0f};
    if (fdata == nullptr) return magenta;

    x = clamp(x, 0, image_width);
    y = clamp(y, 0, image_height);

    return fdata + y * (bytes_per_scanline / sizeof(float)) + x * bytes_per_pixel;
  }

  bool is_valid() const { return fdata != nullptr; }

 private:
  const int bytes_per_pixel = 3;
  float *fdata = nullptr;          // Linear floating point pixel data
  unsigned char *bdata = nullptr;  // Linear 8-bit pixel data
  int image_width = 0;             // Loaded image width
  int image_height = 0;            // Loaded image height
  int bytes_per_scanline = 0;

  static int clamp(int x, int low, int high) {
    // Return the value clamped to the range [low, high).
    if (x < low) return low;
    if (x < high) return x;
    return high - 1;
  }

  static unsigned char float_to_byte(float value) {
    if (value <= 0.0) return 0;
    if (1.0 <= value) return 255;
    return static_cast<unsigned char>(256.0 * value);
  }

  void convert_to_bytes() {
    // Convert the linear floating point pixel data to bytes, storing the
    // resulting byte data in the `bdata` member.

    int total_bytes = image_width * image_height * bytes_per_pixel;
    bdata = new unsigned char[total_bytes];

    // Iterate through all pixel components, converting from [0.0, 1.0] float
    // values to unsigned [0, 255] byte values.

    auto *bptr = bdata;
    auto *fptr = fdata;
    for (auto i = 0; i < total_bytes; i++, fptr++, bptr++) *bptr = float_to_byte(*fptr);
  }
};

// Restore MSVC compiler warnings
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
