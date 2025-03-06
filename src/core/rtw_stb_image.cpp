// Disable strict warnings for this header from the Microsoft Visual C++
// compiler.
#ifdef _MSC_VER
#pragma warning(push, 0)
#endif

#include "rtw_stb_image.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb/stb_image.h"

rtw_image::~rtw_image() {
  delete[] bdata;
  STBI_FREE(fdata);
}

bool rtw_image::load(const std::string &filename) {
  // Loads the linear (gamma=1) image data from the given file name. Returns
  // true if the load succeeded. The resulting data buffer contains the three
  // [0.0, 1.0] floating-point values for the first pixel (red, then green,
  // then blue). Pixels are contiguous, going left to right for the width of
  // the image, followed by the next row below, for the full height of the
  // image.

  auto n = bytes_per_pixel;  // Dummy out parameter: original components per pixel
  fdata = stbi_loadf(filename.c_str(), &image_width, &image_height, &n, bytes_per_pixel);
  if (fdata == nullptr) return false;

  bytes_per_scanline = image_width * bytes_per_pixel;
  convert_to_bytes();
  return true;
}

// Restore MSVC compiler warnings
#ifdef _MSC_VER
#pragma warning(pop)
#endif
