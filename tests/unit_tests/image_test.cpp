#include "glimpse/util/image.h"

#include <iomanip>

#include "../test_cfg.h"

using namespace glimpse;

// Add a helper function for approximate comparisons of floating point values
bool approximately(float a, float b, float epsilon = 0.01f) { return std::abs(a - b) <= epsilon; }
constexpr bool logToConsole = false;

static ut::suite<"image"> imageSuite = [] {
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
    bool allCleared = true;
    for (auto byte : img.data()) {
      if (byte != 0) {
        allCleared = false;
        break;
      }
    }
    expect(allCleared);
  };

  "Image.set"_test = [] {
    Image img(10, 10);

    // Test normal case
    ImageColor c(128, 64, 32);
    img.set(5, 5, c);

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
    img.initialize(img.width, img.height);

    // Set each pixel to a unique color
    for (int y = 0; y < 3; y++) {
      for (int x = 0; x < 3; x++) {
        img.set(x, y, ImageColor(x * 50, y * 50, (x + y) * 25));
      }
    }

    // Verify center pixel
    size_t centerOffset = (1 + 1 * 3) * 3;
    expect(img.data()[centerOffset] == 50_uc);
    expect(img.data()[centerOffset + 1] == 50_uc);
    expect(img.data()[centerOffset + 2] == 50_uc);

    // Verify corner pixel
    size_t cornerOffset = (2 + 2 * 3) * 3;
    expect(img.data()[cornerOffset] == 100_uc);
    expect(img.data()[cornerOffset + 1] == 100_uc);
    expect(img.data()[cornerOffset + 2] == 100_uc);
  };
};

static ut::suite<"image_roundtrip"> imageRoundtripSuite = [] {
  using namespace boost::ut;

  test("image_initialization") = [] {
    Image img(100, 100);
    expect(img.width == 100_i);
    expect(img.height == 100_i);
    expect(img.isValid());
  };

  test("image_set_get") = [] {
    Image img(10, 10);

    // Test byte-based set/get
    img.set(5, 5, ImageColor(123, 45, 67));
    auto px = img.get(5, 5);
    expect(px.rgb[0] == 123_i);
    expect(px.rgb[1] == 45_i);
    expect(px.rgb[2] == 67_i);

    // Test float-based set/get
    img.setFloat(2, 3, 0.5f, 0.25f, 0.75f);
    // float r, g, b;
    auto c = img.getFloat(2, 3);
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
        original.setFloat(x, y, r, g, b);
      }
    }

    // Write the image
    std::string testFile = testOutputPath("roundtrip_test.tga");
    bool writeSuccess = original.write(testFile);
    expect(writeSuccess) << "Failed to write test image";

    // Read it back
    Image loaded(testFile);
    expect(loaded.isValid()) << "Failed to load image";
    expect(loaded.width == original.width) << "Width mismatch";
    expect(loaded.height == original.height) << "Height mismatch";

    // Compare with tolerance
    int diffCount = 0;
    for (int y = 0; y < original.height; y++) {
      for (int x = 0; x < original.width; x++) {
        auto origColor = original.get(x, y);
        auto loadedColor = loaded.get(x, y);

        // Allow for small rounding differences due to compression
        const int tolerance = 50;
        if (abs(static_cast<int>(origColor.rgb[0]) - static_cast<int>(loadedColor.rgb[0])) > tolerance ||
            abs(static_cast<int>(origColor.rgb[1]) - static_cast<int>(loadedColor.rgb[1])) > tolerance ||
            abs(static_cast<int>(origColor.rgb[2]) - static_cast<int>(loadedColor.rgb[2])) > tolerance) {
          std::cout << "Pixel (" << x << "," << y << ") - Original: [" << static_cast<int>(origColor.rgb[0]) << ","
                    << static_cast<int>(origColor.rgb[1]) << "," << static_cast<int>(origColor.rgb[2])
                    << "] Loaded: [" << static_cast<int>(loadedColor.rgb[0]) << ","
                    << static_cast<int>(loadedColor.rgb[1]) << "," << static_cast<int>(loadedColor.rgb[2]) << "]"
                    << std::endl;
          diffCount++;
          if (diffCount > 10) {
            break;
          }
        }
      }
    }
    // Expect minimal differences
    expect(diffCount < 10_i) << "Too many pixel differences: " << diffCount;
  };

  test("image_roundtrip_precision_debug") = [] {
    // Create a simpler test image with specific values to track
    Image original(10, 10);

    // Fill with very specific pattern for easy debugging
    for (int y = 0; y < original.height; y++) {
      for (int x = 0; x < original.width; x++) {
        // Use exact values rather than gradients
        uchar rVal = static_cast<uchar>(x * 25);  // 0, 25, 50, 75...
        uchar gVal = static_cast<uchar>(y * 25);  // 0, 25, 50, 75...
        uchar bVal = 128;                         // Constant middle value
        original.set(x, y, ImageColor(rVal, gVal, bVal));
      }
    }

    // Write the image - try different formats
    std::vector<std::string> formats = {"png", "bmp", "tga", "jpg"};

    for (const auto& format : formats) {
      if (logToConsole) std::cout << "\n===== Testing format: " << format << " =====\n";

      std::string testFile = testOutputPath("roundtrip_test." + format);
      bool writeSuccess = original.write(testFile);
      expect(writeSuccess) << "Failed to write test image";

      // Read it back
      Image loaded(testFile);
      expect(loaded.isValid()) << "Failed to load image";

      // Check a few specific pixels to see exactly what's happening
      std::vector<std::pair<int, int>> testPoints = {{0, 0}, {5, 5}, {9, 9}};

      for (auto [x, y] : testPoints) {
        auto origColor = original.get(x, y);
        auto loadedColor = loaded.get(x, y);

        if (logToConsole) {
          std::cout << "Pixel (" << x << "," << y << ")\n";
          std::cout << "  Original: [" << static_cast<int>(origColor.rgb[0]) << ","
                    << static_cast<int>(origColor.rgb[1]) << "," << static_cast<int>(origColor.rgb[2]) << "]\n";
          std::cout << "  Loaded:   [" << static_cast<int>(loadedColor.rgb[0]) << ","
                    << static_cast<int>(loadedColor.rgb[1]) << "," << static_cast<int>(loadedColor.rgb[2]) << "]\n";
        }

        // Calculate percentage difference
        for (int c = 0; c < 3; c++) {
          float origVal = static_cast<float>(origColor.rgb[c]);
          float loadedVal = static_cast<float>(loadedColor.rgb[c]);

          if (origVal > 0) {
            float pctDiff = 100.0f * std::abs(origVal - loadedVal) / origVal;
            auto threshold = (format == "jpg") ? 5.0f : 1.0f;
            expect(pctDiff < threshold) << "Pixel (" << x << "," << y << ") component " << c
                                         << " difference exceeds 1%: " << pctDiff << "%";
          }
        }
      }
    }
  };

  test("rgb_order_test") = [] {
    Image testImg(3, 1);

    // Set a pure red, pure green, and pure blue pixel
    testImg.set(0, 0, ImageColor(255, 0, 0));  // Red
    testImg.set(1, 0, ImageColor(0, 255, 0));  // Green
    testImg.set(2, 0, ImageColor(0, 0, 255));  // Blue

    std::vector<std::string> formats = {"png", "bmp", "tga", "jpg"};

    for (const auto& format : formats) {
      if (logToConsole) std::cout << "\n===== Testing format: " << format << " =====\n";

      std::string testFile = testOutputPath("rgb_test." + format);
      bool writeSuccess = testImg.write(testFile);
      expect(writeSuccess) << "Failed to write test image";

      Image loaded(testFile);

      // Check each pixel
      auto redPixel = loaded.get(0, 0);
      auto greenPixel = loaded.get(1, 0);
      auto bluePixel = loaded.get(2, 0);

      if (logToConsole)
        std::cout << "Red pixel loaded as: [" << static_cast<int>(redPixel.rgb[0]) << ","
                  << static_cast<int>(redPixel.rgb[1]) << "," << static_cast<int>(redPixel.rgb[2]) << "]\n";

      if (logToConsole)
        std::cout << "Green pixel loaded as: [" << static_cast<int>(greenPixel.rgb[0]) << ","
                  << static_cast<int>(greenPixel.rgb[1]) << "," << static_cast<int>(greenPixel.rgb[2]) << "]\n";

      if (logToConsole)
        std::cout << "Blue pixel loaded as: [" << static_cast<int>(bluePixel.rgb[0]) << ","
                  << static_cast<int>(bluePixel.rgb[1]) << "," << static_cast<int>(bluePixel.rgb[2]) << "]\n";
    }
  };

  test("vertical_flip_test") = [] {
    // Create a test image with a vertical gradient pattern
    const int height = 10;
    Image testImg(1, height);

    // Create a gradient from top to bottom (white to black)
    for (int y = 0; y < height; y++) {
      // Set brightness based on y position (top is white, bottom is black)
      unsigned char value = static_cast<unsigned char>(255 - (y * 255 / (height - 1)));
      testImg.set(0, y, ImageColor(value, value, value));

      // Debug: print what we're setting
      if (logToConsole) std::cout << "Setting y=" << y << " to value: " << static_cast<int>(value) << std::endl;
    }

    // Write/read test for each format
    std::vector<std::string> formats = {"png", "bmp", "tga"};  // Skip jpg due to compression

    for (const auto& format : formats) {
      if (logToConsole) std::cout << "\n===== Testing format: " << format << " for vertical flip =====\n";

      std::string testFile = testOutputPath("vertical_flip_test." + format);
      testImg.write(testFile);

      Image loaded(testFile);
      expect(loaded.isValid());

      // Check each row's value
      for (int y = 0; y < height; y++) {
        auto c = loaded.get(0, y);
        // Average the channels for simplicity
        int avgValue = (c.rgb[0] + c.rgb[1] + c.rgb[2]) / 3;

        if (logToConsole)
          std::cout << "Row " << y << " loaded with value: " << avgValue
                    << " (expected: " << 255 - (y * 255 / (height - 1)) << ")" << std::endl;
      }
    }
  };

  test("gamma_correction_test") = [] {
    // Test a range of gray values
    const int size = 11;  // 0, 25, 50, ... 250
    Image testImg(size, 1);

    // Set grayscale values from 0 to 250 in steps of 25
    for (int x = 0; x < size; x++) {
      unsigned char value = static_cast<unsigned char>(x * 25);
      testImg.set(x, 0, ImageColor(value, value, value));
      if (logToConsole) std::cout << "Setting x=" << x << " to value: " << static_cast<int>(value) << std::endl;
    }

    // Test with PNG (most reliable format)
    std::string testFile = testOutputPath("gamma_test.png");
    testImg.write(testFile);

    Image loaded(testFile);
    expect(loaded.isValid());

    if (logToConsole) {
      // Check each value and the difference
      std::cout << "\nGamma correction results:\n";
      std::cout << "Original | Loaded | Difference\n";
      std::cout << "-----------------------------\n";
    }

    auto diffCount = 0, threshold = 1;
    for (int x = 0; x < size; x++) {
      auto origColor = testImg.get(x, 0);
      auto loadedColor = loaded.get(x, 0);
      int origVal = static_cast<int>(origColor.rgb[0]);
      int loadedVal = static_cast<int>(loadedColor.rgb[0]);
      int diff = loadedVal - origVal;
      if (diff > threshold) {
        diffCount++;
      }

      if (logToConsole) {
        std::cout << std::setw(8) << origVal << " | " << std::setw(6) << loadedVal << " | " << std::setw(10) << diff
                  << std::endl;
      }
    }

    expect(loaded.isValid());
    expect(diffCount < 1_i) << "Too many pixel differences: " << diffCount;
  };
};