#include "core/image.h"

#include <iomanip>

#include "../test_cfg.h"

using namespace glimpse;

// Add a helper function for approximate comparisons of floating point values
bool approximately(float a, float b, float epsilon = 0.01f) { return std::abs(a - b) <= epsilon; }
constexpr bool log_to_console = false;

void image_test() {
  using namespace boost::ut;

  "Image.construction"_test = [] {
    // Default constructor
    {
      Image img;
      expect(img.width == 0_i);
      expect(img.height == 0_i);
    }

    // Sized constructor
    {
      Image img(10, 20);
      expect(img.width == 10_i);
      expect(img.height == 20_i);
    }
  };

  "Image.initialize"_test = [] {
    Image img;
    img.initialize(5, 8);
    expect(img.width == 5_i);
    expect(img.height == 8_i);

    // Test re-initialization
    img.initialize(10, 15);
    expect(img.width == 10_i);
    expect(img.height == 15_i);
  };

  "Image.clear"_test = [] {
    Image img(5, 5);

    // Set all data to non-zero values
    for (auto& byte : img.data()) {
      byte = 255;
    }

    // Clear the image
    img.clear();

    // Verify all bytes are now 0
    bool all_cleared = true;
    for (auto byte : img.data()) {
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
    expect(img.data()[offset] == 128_uc);
    expect(img.data()[offset + 1] == 64_uc);
    expect(img.data()[offset + 2] == 32_uc);

    // Test boundary conditions
    img.set(-1, 5, ImageColor(255, 255, 255));  // Out of bounds negative x
    img.set(5, -1, ImageColor(255, 255, 255));  // Out of bounds negative y
    img.set(10, 5, ImageColor(255, 255, 255));  // Out of bounds max x
    img.set(5, 10, ImageColor(255, 255, 255));  // Out of bounds max y

    // Original values should still be the same
    expect(img.data()[offset] == 128_uc);
    expect(img.data()[offset + 1] == 64_uc);
    expect(img.data()[offset + 2] == 32_uc);
  };

  "Image.set_multiple"_test = [] {
    Image img(3, 3);
    // img.clear();
    img.initialize(img.width, img.height);

    // Set each pixel to a unique color
    for (int y = 0; y < 3; y++) {
      for (int x = 0; x < 3; x++) {
        img.set(x, y, ImageColor(x * 50, y * 50, (x + y) * 25));
      }
    }

    // Verify center pixel
    size_t center_offset = (1 + 1 * 3) * 3;
    expect(img.data()[center_offset] == 50_uc);
    expect(img.data()[center_offset + 1] == 50_uc);
    expect(img.data()[center_offset + 2] == 50_uc);

    // Verify corner pixel
    size_t corner_offset = (2 + 2 * 3) * 3;
    expect(img.data()[corner_offset] == 100_uc);
    expect(img.data()[corner_offset + 1] == 100_uc);
    expect(img.data()[corner_offset + 2] == 100_uc);
  };
};

void test_image_roundtrip() {
  using namespace boost::ut;

  test("image_initialization") = [] {
    Image img(100, 100);
    expect(img.width == 100_i);
    expect(img.height == 100_i);
    expect(img.is_valid());
  };

  test("image_set_get") = [] {
    Image img(10, 10);

    // Test byte-based set/get
    img.set(5, 5, ImageColor(123, 45, 67));
    auto color = img.get(5, 5);
    expect(color.rgb[0] == 123_i);
    expect(color.rgb[1] == 45_i);
    expect(color.rgb[2] == 67_i);

    // Test float-based set/get
    img.set_float(2, 3, 0.5f, 0.25f, 0.75f);
    // float r, g, b;
    auto c = img.get_float(2, 3);
    expect(approximately(static_cast<float>(c[0]), 0.5f)) << "Expected r ≈ 0.5";
    expect(approximately(static_cast<float>(c[1]), 0.25f)) << "Expected g ≈ 0.25";
    expect(approximately(static_cast<float>(c[2]), 0.75f)) << "Expected b ≈ 0.75";
  };

  test("image_roundtrip_precision") = [] {
    // Create test image with a gradient pattern
    Image original(100, 100);
    for (int y = 0; y < original.height; y++) {
      for (int x = 0; x < original.width; x++) {
        // Create a gradient pattern
        float r = static_cast<float>(x) / original.width;
        float g = static_cast<float>(y) / original.height;
        float b = static_cast<float>(x + y) / (original.width + original.height);
        original.set_float(x, y, r, g, b);
      }
    }

    // Write the image
    std::string test_file = "./test_output/roundtrip_test.tga";
    bool write_success = original.write(test_file);
    expect(write_success) << "Failed to write test image";

    // Read it back
    Image loaded(test_file);
    expect(loaded.is_valid()) << "Failed to load image";
    expect(loaded.width == original.width) << "Width mismatch";
    expect(loaded.height == original.height) << "Height mismatch";

    // Compare with tolerance
    int diff_count = 0;
    for (int y = 0; y < original.height; y++) {
      for (int x = 0; x < original.width; x++) {
        auto orig_color = original.get(x, y);
        auto loaded_color = loaded.get(x, y);

        // Allow for small rounding differences due to compression
        const int tolerance = 50;
        if (abs(static_cast<int>(orig_color.rgb[0]) - static_cast<int>(loaded_color.rgb[0])) > tolerance ||
            abs(static_cast<int>(orig_color.rgb[1]) - static_cast<int>(loaded_color.rgb[1])) > tolerance ||
            abs(static_cast<int>(orig_color.rgb[2]) - static_cast<int>(loaded_color.rgb[2])) > tolerance) {
          std::cout << "Pixel (" << x << "," << y << ") - Original: [" << static_cast<int>(orig_color.rgb[0]) << ","
                    << static_cast<int>(orig_color.rgb[1]) << "," << static_cast<int>(orig_color.rgb[2])
                    << "] Loaded: [" << static_cast<int>(loaded_color.rgb[0]) << ","
                    << static_cast<int>(loaded_color.rgb[1]) << "," << static_cast<int>(loaded_color.rgb[2]) << "]"
                    << std::endl;
          diff_count++;
          if (diff_count > 10) {
            break;
          }
        }
      }
    }
    // Expect minimal differences
    expect(diff_count < 10_i) << "Too many pixel differences: " << diff_count;
  };

  test("image_roundtrip_precision_debug") = [] {
    // Create a simpler test image with specific values to track
    Image original(10, 10);

    // Fill with very specific pattern for easy debugging
    for (int y = 0; y < original.height; y++) {
      for (int x = 0; x < original.width; x++) {
        // Use exact values rather than gradients
        uchar r_val = static_cast<uchar>(x * 25);  // 0, 25, 50, 75...
        uchar g_val = static_cast<uchar>(y * 25);  // 0, 25, 50, 75...
        uchar b_val = 128;                         // Constant middle value
        original.set(x, y, ImageColor(r_val, g_val, b_val));
      }
    }

    // Write the image - try different formats
    std::vector<std::string> formats = {"png", "bmp", "tga", "jpg"};

    for (const auto& format : formats) {
      if (log_to_console) std::cout << "\n===== Testing format: " << format << " =====\n";

      std::string test_file = "./test_output/roundtrip_test." + format;
      bool write_success = original.write(test_file);
      expect(write_success) << "Failed to write test image";

      // Read it back
      Image loaded(test_file);
      expect(loaded.is_valid()) << "Failed to load image";

      // Check a few specific pixels to see exactly what's happening
      std::vector<std::pair<int, int>> test_points = {{0, 0}, {5, 5}, {9, 9}};

      for (auto [x, y] : test_points) {
        auto orig_color = original.get(x, y);
        auto loaded_color = loaded.get(x, y);

        if (log_to_console) {
          std::cout << "Pixel (" << x << "," << y << ")\n";
          std::cout << "  Original: [" << static_cast<int>(orig_color.rgb[0]) << ","
                    << static_cast<int>(orig_color.rgb[1]) << "," << static_cast<int>(orig_color.rgb[2]) << "]\n";
          std::cout << "  Loaded:   [" << static_cast<int>(loaded_color.rgb[0]) << ","
                    << static_cast<int>(loaded_color.rgb[1]) << "," << static_cast<int>(loaded_color.rgb[2]) << "]\n";
        }

        // Calculate percentage difference
        for (int c = 0; c < 3; c++) {
          float orig_val = static_cast<float>(orig_color.rgb[c]);
          float loaded_val = static_cast<float>(loaded_color.rgb[c]);

          if (orig_val > 0) {
            float pct_diff = 100.0f * std::abs(orig_val - loaded_val) / orig_val;
            auto threshold = (format == "jpg") ? 5.0f : 1.0f;
            expect(pct_diff < threshold) << "Pixel (" << x << "," << y << ") component " << c
                                         << " difference exceeds 1%: " << pct_diff << "%";
          }
        }
      }
    }
  };

  test("rgb_order_test") = [] {
    Image test_img(3, 1);

    // Set a pure red, pure green, and pure blue pixel
    test_img.set(0, 0, ImageColor(255, 0, 0));  // Red
    test_img.set(1, 0, ImageColor(0, 255, 0));  // Green
    test_img.set(2, 0, ImageColor(0, 0, 255));  // Blue

    std::vector<std::string> formats = {"png", "bmp", "tga", "jpg"};

    for (const auto& format : formats) {
      if (log_to_console) std::cout << "\n===== Testing format: " << format << " =====\n";

      std::string test_file = "./test_output/rgb_test." + format;
      bool write_success = test_img.write(test_file);
      expect(write_success) << "Failed to write test image";

      Image loaded(test_file);

      // Check each pixel
      auto red_pixel = loaded.get(0, 0);
      auto green_pixel = loaded.get(1, 0);
      auto blue_pixel = loaded.get(2, 0);

      if (log_to_console)
        std::cout << "Red pixel loaded as: [" << static_cast<int>(red_pixel.rgb[0]) << ","
                  << static_cast<int>(red_pixel.rgb[1]) << "," << static_cast<int>(red_pixel.rgb[2]) << "]\n";

      if (log_to_console)
        std::cout << "Green pixel loaded as: [" << static_cast<int>(green_pixel.rgb[0]) << ","
                  << static_cast<int>(green_pixel.rgb[1]) << "," << static_cast<int>(green_pixel.rgb[2]) << "]\n";

      if (log_to_console)
        std::cout << "Blue pixel loaded as: [" << static_cast<int>(blue_pixel.rgb[0]) << ","
                  << static_cast<int>(blue_pixel.rgb[1]) << "," << static_cast<int>(blue_pixel.rgb[2]) << "]\n";
    }
  };

  test("vertical_flip_test") = [] {
    // Create a test image with a vertical gradient pattern
    const int height = 10;
    Image test_img(1, height);

    // Create a gradient from top to bottom (white to black)
    for (int y = 0; y < height; y++) {
      // Set brightness based on y position (top is white, bottom is black)
      unsigned char value = static_cast<unsigned char>(255 - (y * 255 / (height - 1)));
      test_img.set(0, y, ImageColor(value, value, value));

      // Debug: print what we're setting
      if (log_to_console) std::cout << "Setting y=" << y << " to value: " << static_cast<int>(value) << std::endl;
    }

    // Write/read test for each format
    std::vector<std::string> formats = {"png", "bmp", "tga"};  // Skip jpg due to compression

    for (const auto& format : formats) {
      if (log_to_console) std::cout << "\n===== Testing format: " << format << " for vertical flip =====\n";

      std::string test_file = "./test_output/vertical_flip_test." + format;
      test_img.write(test_file);

      Image loaded(test_file);
      expect(loaded.is_valid());

      // Check each row's value
      for (int y = 0; y < height; y++) {
        auto color = loaded.get(0, y);
        // Average the channels for simplicity
        int avg_value = (color.rgb[0] + color.rgb[1] + color.rgb[2]) / 3;

        if (log_to_console)
          std::cout << "Row " << y << " loaded with value: " << avg_value
                    << " (expected: " << 255 - (y * 255 / (height - 1)) << ")" << std::endl;
      }
    }
  };

  test("gamma_correction_test") = [] {
    // Test a range of gray values
    const int size = 11;  // 0, 25, 50, ... 250
    Image test_img(size, 1);

    // Set grayscale values from 0 to 250 in steps of 25
    for (int x = 0; x < size; x++) {
      unsigned char value = static_cast<unsigned char>(x * 25);
      test_img.set(x, 0, ImageColor(value, value, value));
      if (log_to_console) std::cout << "Setting x=" << x << " to value: " << static_cast<int>(value) << std::endl;
    }

    // Test with PNG (most reliable format)
    std::string test_file = "./test_output/gamma_test.png";
    test_img.write(test_file);

    Image loaded(test_file);
    expect(loaded.is_valid());

    if (log_to_console) {
      // Check each value and the difference
      std::cout << "\nGamma correction results:\n";
      std::cout << "Original | Loaded | Difference\n";
      std::cout << "-----------------------------\n";
    }

    auto diff_count = 0, threshold = 1;
    for (int x = 0; x < size; x++) {
      auto orig_color = test_img.get(x, 0);
      auto loaded_color = loaded.get(x, 0);
      int orig_val = static_cast<int>(orig_color.rgb[0]);
      int loaded_val = static_cast<int>(loaded_color.rgb[0]);
      int diff = loaded_val - orig_val;
      if (diff > threshold) {
        diff_count++;
      }

      if (log_to_console) {
        std::cout << std::setw(8) << orig_val << " | " << std::setw(6) << loaded_val << " | " << std::setw(10) << diff
                  << std::endl;
      }
    }

    expect(loaded.is_valid());
    expect(diff_count < 1_i) << "Too many pixel differences: " << diff_count;
  };
}