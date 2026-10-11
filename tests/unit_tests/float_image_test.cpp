#include "glimpse/util/float_image.h"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>

//
#include "../test_cfg.h"

using namespace glimpse;

namespace {

// A 3x2 image with distinct, HDR-range values: pixel (x, y) channel c = 100*y + 10*x + c + 0.25.
FloatImage makeTestImage() {
  FloatImage img(3, 2);
  for (int y = 0; y < img.height; ++y)
    for (int x = 0; x < img.width; ++x)
      for (int c = 0; c < 3; ++c) img.rgb[img.index(x, y) + c] = 100.0f * y + 10.0f * x + c + 0.25f;
  return img;
}

std::string readBytes(const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

void writeBytes(const std::string& path, const std::string& bytes) {
  std::ofstream(path, std::ios::binary).write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
}

std::string floatBytes(float f, bool swap) {
  char b[4];
  std::memcpy(b, &f, 4);
  if (swap) std::swap(b[0], b[3]), std::swap(b[1], b[2]);
  return std::string(b, 4);
}

}  // namespace

static ut::suite<"float_image"> floatImageSuite = [] {
  using namespace boost::ut;

  "pfm"_test = [] {
    "round_trip_is_exact"_test = [] {
      const std::string path = testOutputPath("roundtrip.pfm");
      const FloatImage img = makeTestImage();
      expect(writePFM(path, img));
      const auto back = readPFM(path);
      expect(back.has_value() >> fatal);
      expect(back->width == 3_i && back->height == 2_i);
      expect(back->rgb == img.rgb) << "PFM is lossless: every float comes back bit-identical";
    };

    "header_is_PF_width_height_negative_scale"_test = [] {
      const std::string path = testOutputPath("header.pfm");
      expect(writePFM(path, makeTestImage()));
      const std::string bytes = readBytes(path);
      expect(bytes.rfind("PF\n3 2\n-1", 0) == 0) << "negative scale = little-endian floats";
      expect(bytes.size() == bytes.find('\n', bytes.find("-1")) + 1 + 3 * 2 * 3 * 4) << "header + 18 floats";
    };

    // PFM stores scanlines bottom-to-top: the first row in the file is the image's bottom row (row height-1).
    "first_scanline_in_file_is_the_bottom_row"_test = [] {
      const std::string path = testOutputPath("rows.pfm");
      expect(writePFM(path, makeTestImage()));
      const std::string bytes = readBytes(path);
      const size_t data = bytes.find('\n', bytes.find("-1")) + 1;
      float first;
      std::memcpy(&first, bytes.data() + data, 4);
      expect(first == 100.25_f) << "pixel (0, 1), the bottom row, comes first";
    };

    "reads_big_endian_and_scale"_test = [] {
      const std::string path = testOutputPath("big_endian.pfm");
      // 1x1 RGB, big-endian (positive scale), |scale| = 2 multiplies the stored values.
      writeBytes(path, "PF\n1 1\n2.0\n" + floatBytes(1.0f, true) + floatBytes(2.0f, true) + floatBytes(3.0f, true));
      const auto img = readPFM(path);
      expect(img.has_value() >> fatal);
      expect(img->rgb[0] == 2.0_f && img->rgb[1] == 4.0_f && img->rgb[2] == 6.0_f);
    };

    "reads_grayscale_Pf_as_rgb"_test = [] {
      const std::string path = testOutputPath("gray.pfm");
      writeBytes(path, "Pf\n2 1\n-1.0\n" + floatBytes(0.5f, false) + floatBytes(7.0f, false));
      const auto img = readPFM(path);
      expect(img.has_value() >> fatal);
      expect(img->width == 2_i && img->height == 1_i);
      expect(img->rgb[0] == 0.5_f && img->rgb[1] == 0.5_f && img->rgb[2] == 0.5_f);
      expect(img->rgb[3] == 7.0_f && img->rgb[5] == 7.0_f);
    };

    "rejects_missing_truncated_and_garbage"_test = [] {
      expect(!readPFM(testOutputPath("does_not_exist.pfm")).has_value());
      const std::string trunc = testOutputPath("truncated.pfm");
      writeBytes(trunc, "PF\n2 2\n-1.0\n" + floatBytes(1.0f, false));  // needs 12 floats
      expect(!readPFM(trunc).has_value());
      const std::string garbage = testOutputPath("garbage.pfm");
      writeBytes(garbage, "P6\n2 2\n255\n");
      expect(!readPFM(garbage).has_value());
    };
  };
};
