
#include <future>

#include "core/logger.h"

#include "camera.h"
#include "image.h"
#include "scenes.h"
#include "cli.h"

#ifndef ROOT
#define ROOT "."
#endif // !ROOT

const std::string log_file_path = ROOT "/log.txt";
color ray_color(const ray &r, const color &background, const hittable &world,
                int depth);

void render_section(Image &image, int start_row, int end_row, int image_width,
                    int image_height, int samples_per_pixel, const camera &cam,
                    const color &background, const bvh_node &world_bvh,
                    int max_depth)

{
  for (int j = end_row - 1; j >= start_row; --j)
  {
    std::cout << "\rScanlines remaining: " << j << ' ' << std::flush;
    for (int i = 0; i < image_width; ++i)
    {
      color pixel_color(0, 0, 0);
      for (int s = 0; s < samples_per_pixel; ++s)
      {
        auto u = (i + random_double()) / (image_width - 1);
        auto v = (j + random_double()) / (image_height - 1);
        ray r = cam.get_ray(u, v);
        pixel_color += ray_color(r, background, world_bvh, max_depth);
      }
      pixel_color = sqrt(pixel_color / samples_per_pixel);
      image.set(i, j, pixel_color);
    }
  }
}

color ray_color(const ray &r, const color &background, const hittable &world,
                int depth)

{

  hit_record rec;

  // if we've exceeded the ray bounce limit, no more light is gathered
  if (depth < 0)
    return color(0, 0, 0);

  // If the ray hits nothing, return the background color.
  if (!world.hit(r, 0.001, infinity, rec))
    return background;

  ray scattered;
  color attenuation;
  color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);

  if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
    return emitted;

  return emitted +
         attenuation * ray_color(scattered, background, world, depth - 1);

  // vec3 unit_direction = unit_vector(r.direction());
  // auto t = 0.5 * (unit_direction.y() + 1.0);
  // return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

int main(int argc, char **argv)
{
  Logger logger(log_file_path);
  logger.log("Starting...");
  auto options = ParseCommandLine(argc, argv);

  logger.log("Width: ", options.width);
  logger.log("Height: ", options.height);
  logger.log("Samples: ", options.samples);
  logger.log("Max Depth: ", options.max_depth);
  logger.log("Scene: ", options.scene);

  return 0;

  // Image
  // TODO: Parameterize image width and height from cli
  auto aspect_ratio = 16.0 / 9.0;
  int image_width = 500;
  // const int image_width = 10;
  int samples_per_pixel = 100;
  const int max_depth = 50;

  // World
  hittable_list world;

  point3 lookfrom;
  point3 lookat;
  auto vfov = 40.0;
  auto aperture = 0.0;
  color background(0, 0, 0);

  // TODO: Parameterize Scene from cli
  switch (2)
  {
  case 1:
    world = random_scene();
    background = color(0.7, 0.8, 1.0);
    lookfrom = point3(13, 2, 3);
    lookat = point3(0, 0, 0);
    vfov = 20.0;
    aperture = 0.1;
    break;

  default:
  case 2:
    world = two_spheres();
    background = color(0.7, 0.8, 1.0);
    lookfrom = point3(13, 2, 3);
    lookat = point3(0, 0, 0);
    vfov = 20.0;
    break;
  case 3:
    world = two_perlin_spheres();
    background = color(0.7, 0.8, 1.0);
    lookfrom = point3(13, 2, 3);
    lookat = point3(0, 0, 0);
    vfov = 20.0;
    break;
  case 4:
    world = earth();
    background = color(0.7, 0.8, 1.0);
    lookfrom = point3(13, 2, 3);
    lookat = point3(0, 0, 0);
    vfov = 20.0;
    break;
  case 5:
    world = simple_light();
    samples_per_pixel = 400;
    background = color(0, 0, 0);
    lookfrom = point3(26, 3, 6);
    lookat = point3(0, 2, 0);
    vfov = 20.0;
    break;
  case 6:
    world = cornell_box();
    aspect_ratio = 1.0;
    image_width = 600;
    samples_per_pixel = 200;
    background = color(0, 0, 0);
    lookfrom = point3(278, 278, -800);
    lookat = point3(278, 278, 0);
    vfov = 40.0;
    break;
  case 7:
    world = cornell_smoke();
    aspect_ratio = 1.0;
    image_width = 600;
    samples_per_pixel = 200;
    lookfrom = point3(278, 278, -800);
    lookat = point3(278, 278, 0);
    vfov = 40.0;
    break;
  case 8:
    world = final_scene();
    aspect_ratio = 1.0;
    image_width = 800;
    samples_per_pixel = 10000;
    background = color(0, 0, 0);
    lookfrom = point3(478, 278, -600);
    lookat = point3(278, 278, 0);
    vfov = 40.0;
    break;
  }

  auto world_bvh = bvh_node(world, 0, 1);

  // Camera

  vec3 vup(0, 1, 0);
  auto dist_to_focus = 10.0;
  const int image_height = static_cast<int>(image_width / aspect_ratio);

  camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus,
             0.0, 1.0);

  // Render
  logger.log("Rendering... ", image_width, "x", image_height, " with ",
             samples_per_pixel, " samples per pixel");
  auto startTime = std::chrono::high_resolution_clock::now();

  Image image(image_width, image_height);

#define MULTITHREADED
#ifdef MULTITHREADED
  const int num_threads = std::thread::hardware_concurrency();
  std::vector<std::future<void>> futures;
  int rows_per_thread = image_height / num_threads;

  for (int t = 0; t < num_threads; ++t)
  {
    int start_row = t * rows_per_thread;
    int end_row = (t == num_threads - 1) ? image_height : start_row + rows_per_thread;

    // std::async(std::launch::async, render_section, std::ref(image), start_row, end_row, image_width, image_height, samples_per_pixel, std::ref(cam), std::ref(background), std::ref(world_bvh), max_depth);
    futures.push_back(
        std::async(std::launch::async, render_section,
                   std::ref(image), start_row, end_row,
                   image_width, image_height,
                   samples_per_pixel,
                   std::ref(cam), std::ref(background),
                   std::ref(world_bvh), max_depth));
  }

  for (auto &f : futures)
  {
    f.get();
  }
#else // SINGLETHREADED

  // Image image(image_width, image_height);
  render_section(image, 0, image_height, image_width, image_height,
                 samples_per_pixel, cam, background, world_bvh, max_depth);

#endif

  auto endTime = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration<float, std::chrono::seconds::period>(
                      endTime - startTime)
                      .count();

  logger.log("Image generated in ", duration, " seconds");

  std::string result_path(ROOT "/results/final21.jpg");
  logger.log("Writing result to :: ", result_path);
  if (image.write(result_path) != 0)
  {
    logger.log("Image written to ", result_path);
  }
  else
  {
    logger.log("Failed to write image to ", result_path);
  }
}