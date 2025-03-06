#include <filesystem>
#include <fstream>
#include <string>

#include "core/image.h"
#include "core/render.h"
#include "core/scenes.h"
#include "test_scenes.h"

#define BOOST_UT_DISABLE_MODULE
#include "boost/ut.hpp"  // import boost.ut;

namespace fs = std::filesystem;

// Compares two images pixel by pixel with optional tolerance
bool compare_images(const Image& img1, const Image& img2, int tolerance = 1) {
  if (img1.width != img2.width || img1.height != img2.height) {
    std::cout << "Image dimensions don't match: "
              << "(" << img1.width << "x" << img1.height << ") vs "
              << "(" << img2.width << "x" << img2.height << ")" << std::endl;
    return false;
  }

  int total_diff = 0;
  int max_diff = 0;

  for (int y = 0; y < img1.height; y++) {
    for (int x = 0; x < img1.width; x++) {
      ImageColor c1 = img1.get(x, y);
      ImageColor c2 = img2.get(x, y);

      int r_diff = std::abs(static_cast<int>(c1.rgb[0]) - static_cast<int>(c2.rgb[0]));
      int g_diff = std::abs(static_cast<int>(c1.rgb[1]) - static_cast<int>(c2.rgb[1]));
      int b_diff = std::abs(static_cast<int>(c1.rgb[2]) - static_cast<int>(c2.rgb[2]));

      int pixel_diff = std::max({r_diff, g_diff, b_diff});
      max_diff = std::max(max_diff, pixel_diff);

      if (pixel_diff > tolerance) {
        total_diff++;
      }
    }
  }

  if (total_diff > 0) {
    std::cout << "Images differ by " << total_diff << " pixels (max diff: " << max_diff << ")" << std::endl;
    return false;
  }

  return true;
}

// Creates directory if it doesn't exist
void ensure_directory_exists(const std::string& path) {
  if (!fs::exists(path)) {
    fs::create_directories(path);
  }
}

// Load reference image
Image load_reference_image(const std::string& filename) {
  Image img;
  // TODO:: Load reference image
  //  if (!img.read(filename)) {
  //    std::cout << "Failed to load reference image: " << filename << std::endl;
  //  }
  return img;
}

// Run end-to-end test for a single scene
void test_scene(const std::string& name, Scene (*create_scene_fn)(), int width = 200, int height = 0) {
  using namespace boost::ut;

  const std::string test_output_dir = "./test_output/";
  const std::string reference_dir = "./tests/reference_images/";

  ensure_directory_exists(test_output_dir);

  // Create test image filename
  std::string output_filename = test_output_dir + name + ".png";
  std::string reference_filename = reference_dir + name + ".png";

  // std::string test_name = ;
  test("e2e_" + name + "_render") = [&] {
    // Create scene
    Scene scene = create_scene_fn();

    // Override dimensions for testing
    scene.cam.image_width = width;
    if (height > 0) {
      scene.cam.image_height = height;
    } else {
      // Calculate height from aspect ratio
      scene.cam.image_height = static_cast<int>(width / scene.cam.aspect_ratio);
    }

    // Reduce samples for faster testing
    scene.cam.samples_per_pixel = 4;  // Low number for quick tests
    scene.cam.max_depth = 4;          // Reduced bounce depth

    scene.cam.initialize();

    // Create and initialize image
    Image rendered_image(scene.cam.image_width, scene.cam.image_height);

    // Render
    std::cout << "Rendering test scene: " << name << " (" << scene.cam.image_width << "x" << scene.cam.image_height
              << ")"
              << " with " << scene.cam.samples_per_pixel << " samples" << std::endl;

    std::atomic<int> progress = 0;
    Renderer renderer;
    renderer.render_scene(scene, rendered_image, &progress);

    // Save rendered image
    bool write_success = rendered_image.write(output_filename);
    expect(write_success) << "Failed to write output image to " << output_filename;

    // If reference image exists, compare
    if (fs::exists(reference_filename)) {
      Image reference_image = load_reference_image(reference_filename);
      bool images_match = compare_images(rendered_image, reference_image);
      expect(images_match) << "Rendered image doesn't match reference for scene: " << name;
    } else {
      std::cout << "No reference image found at: " << reference_filename
                << ". Created new output as potential reference." << std::endl;
    }
  };
}

void e2e_test() {
  // Test each scene
  test_scene("simple_sphere", create_simple_sphere_scene);
  test_scene("cornell_box", create_cornell_box_scene, 200, 200);  // Square aspect ratio
  test_scene("reflective_sphere", create_reflective_sphere_scene);
  // Add more scenes as needed
}