#include <filesystem>
#include <fstream>
#include <future>
#include <string>

#include "core/image.h"
#include "core/render.h"
#include "core/scenes.h"
#include "print_ascii_image.h"
#include "test_scenes.h"

//
#include "../test_cfg.h"

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

// Run end-to-end test for a scene with uncapped samples per pixel
void test_scene_uncapped(const std::string& name, Scene (*create_scene_fn)(), int width = 200, int height = 0,
                         int run_time_seconds = 5) {
  using namespace boost::ut;

  const std::string test_output_dir = "./test_output/";
  const std::string reference_dir = "./tests/e2e/reference_images/";

  ensure_directory_exists(test_output_dir);

  // Create test image filename
  std::string output_filename = test_output_dir + name + "_uncapped.png";

  test("e2e_" + name + "_uncapped_render") = [&, run_time_seconds] {
    // Create scene
    Scene scene = create_scene_fn();

    // Setup for uncapped SPP mode
    scene.cam.uncapped_spp = true;

    // Override dimensions for testing
    scene.cam.image_width = width;
    if (height > 0) {
      scene.cam.image_height = height;
    } else {
      // Calculate height from aspect ratio
      scene.cam.image_height = static_cast<int>(width / scene.cam.aspect_ratio);
    }

    // Lower depth for testing
    scene.cam.max_depth = 4;

    scene.cam.initialize();

    // Create and initialize image
    Image rendered_image(scene.cam.image_width, scene.cam.image_height);

    // Render for specified duration
    std::cout << "Rendering uncapped test scene: " << name << " (" << scene.cam.image_width << "x"
              << scene.cam.image_height << ")"
              << " for " << run_time_seconds << " seconds" << std::endl;

    std::atomic<int> progress = 0;
    Renderer renderer;

    // Start rendering in a separate thread
    auto render_future =
        std::async(std::launch::async, [&]() { renderer.render_scene(scene, rendered_image, &progress); });

    // Let it run for specified time
    std::this_thread::sleep_for(std::chrono::seconds(run_time_seconds));

    // Signal renderer to stop
    Renderer::stop_rendering = true;

    // Wait for renderer to finish
    render_future.wait();

    // Reset flag for future tests
    Renderer::stop_rendering = false;

    // Save rendered image
    bool write_success = rendered_image.write(output_filename);
    expect(write_success) << "Failed to write uncapped output image to " << output_filename;

    std::cout << "Completed uncapped rendering test for " << name << std::endl;
  };
}

void test_debug_scene() {
  using namespace boost::ut;

  const std::string test_output_dir = "./test_output/";
  ensure_directory_exists(test_output_dir);
  std::string output_filename = test_output_dir + "debug_scene.png";

  test("e2e_debug_scene") = [&] {
    // Create the debug scene
    Scene scene = create_debug_scene();

    // Create and initialize image
    Image rendered_image(scene.cam.image_width, scene.cam.image_height);

    std::cout << "Rendering debug scene (" << scene.cam.image_width << "x" << scene.cam.image_height << ")"
              << std::endl;

    // Render synchronously (no need for async with tiny image)
    std::atomic<int> progress = 0;
    Renderer renderer;
    renderer.render_scene(scene, rendered_image, &progress);

    // Save rendered image
    bool write_success = rendered_image.write(output_filename);
    expect(write_success) << "Failed to write debug scene image";

    // Print the pixel values for manual verification
    std::cout << "Debug scene pixel values:" << std::endl;
    for (int y = 0; y < scene.cam.image_height; y++) {
      for (int x = 0; x < scene.cam.image_width; x++) {
        ImageColor c = rendered_image.get(x, y);
        std::cout << "Pixel (" << x << "," << y << "): [" << static_cast<int>(c.rgb[0]) << ","
                  << static_cast<int>(c.rgb[1]) << "," << static_cast<int>(c.rgb[2]) << "]" << std::endl;
      }
    }

    // print_ascii_image(rendered_image);
    print_color_ascii_image(rendered_image);
    // print_enhanced_ascii_image(rendered_image);
    // print_feature_visualization(rendered_image);

    // Create a known reference image manually for comparison
    Image reference_image(scene.cam.image_width, scene.cam.image_height);

    // Center pixels should be red from the sphere, outer pixels should be blue background
    // We'll estimate a basic pattern
    for (int y = 0; y < scene.cam.image_height; y++) {
      for (int x = 0; x < scene.cam.image_width; x++) {
        // Simplified: Center pixels are red, outer pixels are blue
        double center_x = scene.cam.image_width / 2.0 - 0.5;
        double center_y = scene.cam.image_height / 2.0 - 0.5;
        double distance = sqrt(pow(x - center_x, 2) + pow(y - center_y, 2));

        if (distance < 1.5) {
          // Red sphere at the center
          reference_image.set(x, y,
                              ImageColor(static_cast<unsigned char>(255), static_cast<unsigned char>(0),
                                         static_cast<unsigned char>(0)));
        } else {
          // Blue background
          reference_image.set(x, y,
                              ImageColor(static_cast<unsigned char>(128), static_cast<unsigned char>(179),
                                         static_cast<unsigned char>(255)));  // Approximate blue background
        }
      }
    }

    // Save reference image
    std::string ref_output = test_output_dir + "debug_scene_reference.png";
    reference_image.write(ref_output);

    // Compare images
    bool images_match = compare_images(rendered_image, reference_image, 100);  // Higher tolerance for this test

    // If they don't match, print specific differences
    if (!images_match) {
      std::cout << "Debug scene pixel differences:" << std::endl;
      for (int y = 0; y < scene.cam.image_height; y++) {
        for (int x = 0; x < scene.cam.image_width; x++) {
          ImageColor rendered = rendered_image.get(x, y);
          ImageColor reference = reference_image.get(x, y);

          int r_diff = abs(static_cast<int>(rendered.rgb[0]) - static_cast<int>(reference.rgb[0]));
          int g_diff = abs(static_cast<int>(rendered.rgb[1]) - static_cast<int>(reference.rgb[1]));
          int b_diff = abs(static_cast<int>(rendered.rgb[2]) - static_cast<int>(reference.rgb[2]));

          if (r_diff > 10 || g_diff > 10 || b_diff > 10) {
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
void test_scene(const std::string& name, Scene (*create_scene_fn)(), int width = 200, int height = 0,
                unsigned int seed = 42) {
  using namespace boost::ut;

  const std::string test_output_dir = "./test_output/";
  const std::string reference_dir = "./tests/e2e/reference_images/";

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

    // Set the random seed for deterministic rendering
    Random::set_seed(seed);

    scene.cam.initialize();

    // Create and initialize image
    Image rendered_image(scene.cam.image_width, scene.cam.image_height);

    // Render
    std::cout << "Rendering test scene: " << name << " (" << scene.cam.image_width << "x" << scene.cam.image_height
              << ")"
              << " with " << scene.cam.samples_per_pixel << " samples" << std::endl;

    std::atomic<int> progress = 0;
    std::atomic<bool> render_completed = false;
    Renderer renderer;
    renderer.render_scene(scene, rendered_image, &progress);

    // start render in a sperate thread!
    auto render_future = std::async(std::launch::async, [&]() {
      renderer.render_scene(scene, rendered_image, &progress);
      render_completed = true;
    });

    // Wait for rendering to complete with a timeout
    // TODO: Find a reasonable timeout value, 10 is too much
    const int timeout_seconds = 10;
    auto start_time = std::chrono::steady_clock::now();
    bool timed_out = false;
    while (!render_completed) {
      // wait...
      std::this_thread::sleep_for(std::chrono::milliseconds(100));

      auto elapsed_seconds =
          std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_time).count();
      if (elapsed_seconds > timeout_seconds) {
        timed_out = true;
        std::cout << "Rendering timed out after " << timeout_seconds << " seconds" << std::endl;

        Renderer::stop_rendering = true;  // Stop rendering

        break;
      }
    }

    // TODO: correct image flip behaviour
    //  TODO: find a way to fix random seed to check for correctness!!

    if (!timed_out) {
      // Wait for rendering to finish if it hasnt already,
      // TODO, maybe this should be an assert? since we expect the render to finish
      // TODO, also compare the total elapsed time with a reference value to compare changes in performance
      render_future.wait();
      std::cout << "Rendering completed succesfully." << std::endl;
    } else {
      // Force wait for the rendering thread to clean up
      render_future.wait();
      std::cout << "Proceeding with comparison after forced stop." << std::endl;
    }

    // Reset the global stop flag for future tests!
    // TODO: Maybe this shouldnt be a global state afterall,
    // or maybe it should be reset in the renderer instance destructor itself
    Renderer::stop_rendering = false;

    // Save rendered image
    bool write_success = rendered_image.write(output_filename);
    expect(write_success) << "Failed to write output image to " << output_filename;

    // std::cout << "ASCII preview of rendered image:" << std::endl;
    // print_color_ascii_image(rendered_image);
    // print_truecolor_image(rendered_image);
    // print_pixel_image(rendered_image);
    // print_best_image(rendered_image);
    // print_highres_image(rendered_image);
    // print_ultrahd_console_image(rendered_image, 60, 30);
    // print_enhanced_ascii_image(rendered_image);

    // If reference image exists, compare
    if (fs::exists(reference_filename)) {
      Image reference_image(reference_filename);
      bool images_match = compare_images(rendered_image, reference_image, 10);
      expect(images_match) << "Rendered image doesn't match reference for scene: " << name;
    } else {
      std::cout << "No reference image found at: " << reference_filename
                << ". Created new output as potential reference." << std::endl;
    }
  };
}

// TODO: Test the uncapped spp render branch

void e2e_test() {
  // test_debug_scene();

  // Test each scene
  // test_scene("simple_sphere", create_simple_sphere_scene);
  // test_scene("cornell_box", create_cornell_box_scene, 200, 200);  // Square aspect ratio
  test_scene("reflective_sphere", create_reflective_sphere_scene);
  // Add more scenes as needed

  // test_scene_uncapped("simple_sphere", create_simple_sphere_scene, 200, 0, 3);
  // test_scene_uncapped("cornell_box", create_cornell_box_scene, 200, 0, 3);
  // test_scene_uncapped("reflective_sphere", create_reflective_sphere_scene);
}