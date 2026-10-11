#include <filesystem>
#include <fstream>
#include <future>
#include <string>

#include "glimpse/util/image.h"
#include "glimpse/render.h"
#include "glimpse/scene.h"
#include "print_ascii_image.h"
#include "test_scenes.h"

//
#include "../test_cfg.h"

namespace fs = std::filesystem;
constexpr const char* extForTesting = ".png";

// Compares two images pixel by pixel with optional tolerance
bool compareImages(const Image& img1, const Image& img2, int tolerance = 1, Image* diffImage = nullptr) {
  if (img1.width != img2.width || img1.height != img2.height) {
    std::cout << "Image dimensions don't match: "
              << "(" << img1.width << "x" << img1.height << ") vs "
              << "(" << img2.width << "x" << img2.height << ")" << std::endl;
    return false;
  }

  int totalDiff = 0;
  int maxDiff = 0;

  for (int y = 0; y < img1.height; y++) {
    for (int x = 0; x < img1.width; x++) {
      ImageColor c1 = img1.get(x, y);
      ImageColor c2 = img2.get(x, y);

      int rDiff = std::abs(static_cast<int>(c1.rgb[0]) - static_cast<int>(c2.rgb[0]));
      int gDiff = std::abs(static_cast<int>(c1.rgb[1]) - static_cast<int>(c2.rgb[1]));
      int bDiff = std::abs(static_cast<int>(c1.rgb[2]) - static_cast<int>(c2.rgb[2]));

      int pixelDiff = std::max({rDiff, gDiff, bDiff});
      maxDiff = std::max(maxDiff, pixelDiff);

      if (pixelDiff > tolerance) {
        // std::cout << "Pixel (" << x << "," << y << ") differs by " << pixelDiff << " (max: " << tolerance << ")"
        //           << std::endl;
        // std::cout << "  Expected: [" << static_cast<int>(c1.rgb[0]) << "," << static_cast<int>(c1.rgb[1]) << ","
        //           << static_cast<int>(c1.rgb[2]) << "]" << std::endl;
        // std::cout << "  Actual  : [" << static_cast<int>(c2.rgb[0]) << "," << static_cast<int>(c2.rgb[1]) << ","
        //           << static_cast<int>(c2.rgb[2]) << "]" << std::endl;
        totalDiff++;
      }

      if (diffImage != nullptr) {
        if (pixelDiff > tolerance) {
          // Highlight differences in red, with intensity proportional to difference
          int intensity = std::min(255, pixelDiff * 4);  // Amplify differences for visibility
          diffImage->set(x, y, ImageColor(intensity, 0, 0));
        } else {
          // Use grayscale for pixels that are similar enough
          unsigned char gray = static_cast<unsigned char>((c1.rgb[0] + c1.rgb[1] + c1.rgb[2]) / 3);
          diffImage->set(x, y, ImageColor(gray, gray, gray));
        }
      }
    }
  }

  if (totalDiff > 0) {
    std::cout << "Images differ by " << totalDiff << " pixels (max diff: " << maxDiff << ")" << std::endl;
    return false;
  }

  return true;
}

// Creates directory if it doesn't exist
void ensureDirectoryExists(const std::string& path) {
  if (!fs::exists(path)) {
    fs::create_directories(path);
  }
}

// Run end-to-end test for a scene with uncapped samples per pixel
void testSceneUncapped(const std::string& name, Scene (*createSceneFn)(), int width = 200, int height = 0,
                         int runTimeSeconds = 5, int seed = 42) {
  using namespace boost::ut;

  // Set the random seed for deterministic rendering
  Random::setSeed(seed);

  const std::string testOutputDir = testOutputPath("");
  const std::string referenceDir = "./tests/e2e/reference_images/";

  ensureDirectoryExists(testOutputDir);

  // Create test image filename
  std::string outputFilename = testOutputDir + name + "_uncapped" + extForTesting;
  std::string diffFilename = testOutputDir + name + "_uncapped_diff" + extForTesting;
  std::string referenceFilename = referenceDir + name + "_uncapped" + extForTesting;

  test("e2e_" + name + "_uncapped_render") = [&, runTimeSeconds] {
    // Create scene
    Scene scene = createSceneFn();

    // Setup for uncapped SPP mode
    scene.cam.uncappedSpp = true;

    // Override dimensions for testing
    scene.cam.imageWidth = width;
    if (height > 0) {
      scene.cam.imageHeight = height;
    } else {
      // Calculate height from aspect ratio
      scene.cam.imageHeight = static_cast<int>(width / scene.cam.aspectRatio);
    }

    // Lower depth for testing
    scene.cam.maxDepth = 4;

    scene.cam.initialize();

    // Create and initialize image
    Image renderedImage(scene.cam.imageWidth, scene.cam.imageHeight);

    // Render for specified duration
    std::cout << "Rendering uncapped test scene: " << name << " (" << scene.cam.imageWidth << "x"
              << scene.cam.imageHeight << ")"
              << " for " << runTimeSeconds << " seconds" << std::endl;

    std::atomic<int> progress = 0;
    Renderer renderer;

    // Start rendering in a separate thread
    auto renderFuture =
        std::async(std::launch::async, [&]() { renderer.renderScene(scene, renderedImage, &progress); });

    // Let it run for specified time
    std::this_thread::sleep_for(std::chrono::seconds(runTimeSeconds));

    // Signal renderer to stop
    Renderer::stopRequested = true;

    // Wait for renderer to finish
    renderFuture.wait();

    // Reset flag for future tests
    Renderer::stopRequested = false;

    // Save rendered image
    bool writeSuccess = renderedImage.write(outputFilename);
    expect(writeSuccess) << "Failed to write uncapped output image to " << outputFilename;

    if (fs::exists(referenceFilename)) {
      // Load reference image
      Image referenceImage(referenceFilename);

      // Compare images
      bool imagesMatch = compareImages(renderedImage, referenceImage, 10);

      // If they don't match, create a diff image
      if (!imagesMatch) {
        Image diffImage(scene.cam.imageWidth, scene.cam.imageHeight);
        compareImages(renderedImage, referenceImage, 10, &diffImage);
        diffImage.write(diffFilename);
      }
    } else {
      std::cout << "Reference image not found for comparison: " << referenceFilename << std::endl;
    }

    std::cout << "Completed uncapped rendering test for " << name << std::endl;
  };
}

void testDebugScene() {
  using namespace boost::ut;

  const std::string testOutputDir = testOutputPath("");
  ensureDirectoryExists(testOutputDir);
  std::string outputFilename = testOutputDir + "debug_scene" + extForTesting;

  test("e2e_debug_scene") = [&] {
    // Create the debug scene
    Scene scene = createDebugScene();

    // Create and initialize image
    Image renderedImage(scene.cam.imageWidth, scene.cam.imageHeight);

    std::cout << "Rendering debug scene (" << scene.cam.imageWidth << "x" << scene.cam.imageHeight << ")"
              << std::endl;

    // Render synchronously (no need for async with tiny image)
    std::atomic<int> progress = 0;
    Renderer renderer;
    renderer.renderScene(scene, renderedImage, &progress);

    // Save rendered image
    bool writeSuccess = renderedImage.write(outputFilename);
    expect(writeSuccess) << "Failed to write debug scene image";

    // Print the pixel values for manual verification
    std::cout << "Debug scene pixel values:" << std::endl;
    for (int y = 0; y < scene.cam.imageHeight; y++) {
      for (int x = 0; x < scene.cam.imageWidth; x++) {
        ImageColor c = renderedImage.get(x, y);
        std::cout << "Pixel (" << x << "," << y << "): [" << static_cast<int>(c.rgb[0]) << ","
                  << static_cast<int>(c.rgb[1]) << "," << static_cast<int>(c.rgb[2]) << "]" << std::endl;
      }
    }

    // printAsciiImage(renderedImage);
    printColorAsciiImage(renderedImage);
    // printEnhancedAsciiImage(renderedImage);
    // printFeatureVisualization(renderedImage);

    // Create a known reference image manually for comparison
    Image referenceImage(scene.cam.imageWidth, scene.cam.imageHeight);

    // Center pixels should be red from the sphere, outer pixels should be blue background
    // We'll estimate a basic pattern
    for (int y = 0; y < scene.cam.imageHeight; y++) {
      for (int x = 0; x < scene.cam.imageWidth; x++) {
        // Simplified: Center pixels are red, outer pixels are blue
        double centerX = scene.cam.imageWidth / 2.0 - 0.5;
        double centerY = scene.cam.imageHeight / 2.0 - 0.5;
        double distance = sqrt(pow(x - centerX, 2) + pow(y - centerY, 2));

        if (distance < 1.5) {
          // Red sphere at the center
          referenceImage.set(x, y,
                              ImageColor(static_cast<unsigned char>(255), static_cast<unsigned char>(0),
                                         static_cast<unsigned char>(0)));
        } else {
          // Blue background
          referenceImage.set(x, y,
                              ImageColor(static_cast<unsigned char>(128), static_cast<unsigned char>(179),
                                         static_cast<unsigned char>(255)));  // Approximate blue background
        }
      }
    }

    // Save reference image
    std::string refOutput = testOutputDir + "debug_scene_reference" + extForTesting;
    referenceImage.write(refOutput);

    // Compare images
    bool imagesMatch = compareImages(renderedImage, referenceImage, 100);  // Higher tolerance for this test

    // If they don't match, print specific differences
    if (!imagesMatch) {
      std::cout << "Debug scene pixel differences:" << std::endl;
      for (int y = 0; y < scene.cam.imageHeight; y++) {
        for (int x = 0; x < scene.cam.imageWidth; x++) {
          ImageColor rendered = renderedImage.get(x, y);
          ImageColor reference = referenceImage.get(x, y);

          int rDiff = abs(static_cast<int>(rendered.rgb[0]) - static_cast<int>(reference.rgb[0]));
          int gDiff = abs(static_cast<int>(rendered.rgb[1]) - static_cast<int>(reference.rgb[1]));
          int bDiff = abs(static_cast<int>(rendered.rgb[2]) - static_cast<int>(reference.rgb[2]));

          if (rDiff > 10 || gDiff > 10 || bDiff > 10) {
            std::cout << "Pixel (" << x << "," << y << ") - Rendered: [" << static_cast<int>(rendered.rgb[0]) << ","
                      << static_cast<int>(rendered.rgb[1]) << "," << static_cast<int>(rendered.rgb[2])
                      << "] Reference: [" << static_cast<int>(reference.rgb[0]) << ","
                      << static_cast<int>(reference.rgb[1]) << "," << static_cast<int>(reference.rgb[2]) << "]"
                      << std::endl;
          }
        }
      }
    }
  };
}

// Run end-to-end test for a single scene
void testScene(const std::string& name, Scene (*createSceneFn)(), int width = 200, int height = 0,
                unsigned int seed = 42) {
  using namespace boost::ut;

  // Set the random seed for deterministic rendering
  Random::setSeed(seed);

  const std::string testOutputDir = testOutputPath("");
  const std::string referenceDir = "./tests/e2e/reference_images/";

  ensureDirectoryExists(testOutputDir);

  // Create test image filename
  std::string outputFilename = testOutputDir + name + extForTesting;
  std::string referenceFilename = referenceDir + name + extForTesting;
  std::string diffFilename = testOutputDir + name + "_diff" + extForTesting;

  // std::string testName = ;
  test("e2e_" + name + "_render") = [&] {
    // Create scene
    Scene scene = createSceneFn();

    // Override dimensions for testing
    scene.cam.imageWidth = width;
    if (height > 0) {
      scene.cam.imageHeight = height;
    } else {
      // Calculate height from aspect ratio
      scene.cam.imageHeight = static_cast<int>(width / scene.cam.aspectRatio);
    }

    // Reduce samples for faster testing
    scene.cam.samplesPerPixel = 100;  // Low number for quick tests
    scene.cam.maxDepth = 4;            // Reduced bounce depth

    scene.cam.initialize();

    // Create and initialize image
    Image renderedImage(scene.cam.imageWidth, scene.cam.imageHeight);

    // Render
    std::cout << "Rendering test scene: " << name << " (" << scene.cam.imageWidth << "x" << scene.cam.imageHeight
              << ")"
              << " with " << scene.cam.samplesPerPixel << " samples" << std::endl;

    std::atomic<int> progress = 0;
    std::atomic<bool> renderCompleted = false;
    Renderer renderer;
    renderer.renderScene(scene, renderedImage, &progress);

    // start render in a sperate thread!
    auto renderFuture = std::async(std::launch::async, [&]() {
      renderer.renderScene(scene, renderedImage, &progress);
      renderCompleted = true;
    });

    // Wait for rendering to complete with a timeout
    // TODO: Find a reasonable timeout value, 10 is too much
    const int timeoutSeconds = 10;
    auto startTime = std::chrono::steady_clock::now();
    bool timedOut = false;
    while (!renderCompleted) {
      // wait...
      std::this_thread::sleep_for(std::chrono::milliseconds(100));

      auto elapsedSeconds =
          std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - startTime).count();
      if (elapsedSeconds > timeoutSeconds) {
        timedOut = true;
        std::cout << "Rendering timed out after " << timeoutSeconds << " seconds" << std::endl;

        Renderer::stopRequested = true;  // Stop rendering

        break;
      }
    }

    // TODO: correct image flip behaviour
    //  TODO: find a way to fix random seed to check for correctness!!

    if (!timedOut) {
      // Wait for rendering to finish if it hasnt already,
      // TODO, maybe this should be an assert? since we expect the render to finish
      // TODO, also compare the total elapsed time with a reference value to compare changes in performance
      renderFuture.wait();
      std::cout << "Rendering completed succesfully." << std::endl;
    } else {
      // Force wait for the rendering thread to clean up
      renderFuture.wait();
      std::cout << "Proceeding with comparison after forced stop." << std::endl;
    }

    // Reset the global stop flag for future tests!
    // TODO: Maybe this shouldnt be a global state afterall,
    // or maybe it should be reset in the renderer instance destructor itself
    Renderer::stopRequested = false;

    // Save rendered image
    bool writeSuccess = renderedImage.write(outputFilename);
    expect(writeSuccess) << "Failed to write output image to " << outputFilename;

    // std::cout << "ASCII preview of rendered image:" << std::endl;
    // printColorAsciiImage(renderedImage);
    // printTruecolorImage(renderedImage);
    // printPixelImage(renderedImage);
    // printBestImage(renderedImage);
    // printHighresImage(renderedImage);
    // printUltrahdConsoleImage(renderedImage, 60, 30);
    // printEnhancedAsciiImage(renderedImage);

    // If reference image exists, compare
    if (fs::exists(referenceFilename)) {
      Image referenceImage(referenceFilename);

      // create diff image
      Image diffImage(scene.cam.imageWidth, scene.cam.imageHeight);
      bool imagesMatch = compareImages(renderedImage, referenceImage, 10, &diffImage);
      diffImage.write(diffFilename);
      expect(imagesMatch) << "Rendered image doesn't match reference for scene: " << name;
    } else {
      std::cout << "No reference image found at: " << referenceFilename
                << ". Created new output as potential reference." << std::endl;
    }
  };
}

// TODO: Test the uncapped spp render branch

void e2eTest() {
  // testDebugScene();

  // Test each scene
  testScene("simple_sphere", createSimpleSphereScene);
  testScene("cornell_box", createCornellBoxScene, 200, 200);  // Square aspect ratio
  testScene("reflective_sphere", createReflectiveSphereScene);
  // Add more scenes as needed

  testSceneUncapped("simple_sphere", createSimpleSphereScene, 200, 0, 3);
  testSceneUncapped("cornell_box", createCornellBoxScene, 200, 0, 3);
  testSceneUncapped("reflective_sphere", createReflectiveSphereScene);
}