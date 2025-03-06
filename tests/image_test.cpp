#include "core/image.h"

#define BOOST_UT_DISABLE_MODULE
#include "boost/ut.hpp"

void image_test() {
  using namespace boost::ut;

  "Image.construction"_test = [] {
    // Default constructor
    {
      Image img;
      expect(img.width == 0_i);
      expect(img.height == 0_i);
      expect(img.data.size() == 0_u);
    }

    // Sized constructor
    {
      Image img(10, 20);
      expect(img.width == 10_i);
      expect(img.height == 20_i);
      expect(img.data.size() == (10 * 20 * 3));
    }
  };

  "Image.initialize"_test = [] {
    Image img;
    img.initialize(5, 8);
    expect(img.width == 5_i);
    expect(img.height == 8_i);
    expect(img.data.size() == 5 * 8 * 3);

    // Test re-initialization
    img.initialize(10, 15);
    expect(img.width == 10_i);
    expect(img.height == 15_i);
    expect(img.data.size() == 10 * 15 * 3);
  };

  "Image.clear"_test = [] {
    Image img(5, 5);

    // Set all data to non-zero values
    for (auto& byte : img.data) {
      byte = 255;
    }

    // Clear the image
    img.clear();

    // Verify all bytes are now 0
    bool all_cleared = true;
    for (auto byte : img.data) {
      if (byte != 0) {
        all_cleared = false;
        break;
      }
    }
    expect(all_cleared);
  };

  "Image.set"_test = [] {
    Image img(10, 10);

    // Test normal case
    ImageColor color(128, 64, 32);
    img.set(5, 5, color);

    size_t offset = (5 + 5 * 10) * 3;
    expect(img.data[offset] == 128_uc);
    expect(img.data[offset + 1] == 64_uc);
    expect(img.data[offset + 2] == 32_uc);

    // Test boundary conditions
    img.set(-1, 5, ImageColor(255, 255, 255));  // Out of bounds negative x
    img.set(5, -1, ImageColor(255, 255, 255));  // Out of bounds negative y
    img.set(10, 5, ImageColor(255, 255, 255));  // Out of bounds max x
    img.set(5, 10, ImageColor(255, 255, 255));  // Out of bounds max y

    // Original values should still be the same
    expect(img.data[offset] == 128_uc);
    expect(img.data[offset + 1] == 64_uc);
    expect(img.data[offset + 2] == 32_uc);
  };

  "Image.set_multiple"_test = [] {
    Image img(3, 3);
    img.clear();

    // Set each pixel to a unique color
    for (int y = 0; y < 3; y++) {
      for (int x = 0; x < 3; x++) {
        img.set(x, y, ImageColor(x * 50, y * 50, (x + y) * 25));
      }
    }

    // Verify center pixel
    size_t center_offset = (1 + 1 * 3) * 3;
    expect(img.data[center_offset] == 50_uc);
    expect(img.data[center_offset + 1] == 50_uc);
    expect(img.data[center_offset + 2] == 50_uc);

    // Verify corner pixel
    size_t corner_offset = (2 + 2 * 3) * 3;
    expect(img.data[corner_offset] == 100_uc);
    expect(img.data[corner_offset + 1] == 100_uc);
    expect(img.data[corner_offset + 2] == 100_uc);
  };
};