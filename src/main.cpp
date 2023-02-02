
#include "utils.h"
#include "color.h"
#include "image.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"

#include <string>
#include <iostream>
#include <filesystem>


color ray_color(const ray& r, const hittable& world, int depth) {
    hit_record rec;

    // if we've exceeded the ray bounce limit, no more light is gathered
    if(depth < 0) return color(0, 0, 0);

    if(world.hit(r, 0.001, infinity, rec)) {
        ray scattered;
        color attenuation;
        if(rec.mat_ptr->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, world, depth - 1);
        return color(0, 0, 0);
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

int main() {
    
    // Image
    const auto aspect_ratio = 16.0 / 9.0;
    const int image_width = 1000;
    const int image_height = static_cast<int>(image_width / aspect_ratio);
    const int samples_per_pixel = 100;
    const int max_depth = 50;
    Image image(image_width, image_height);

    // World
    hittable_list world;

    auto material_ground = make_shared<lambertian>(color{0.8, 0.8, 0.0});
    auto material_center = make_shared<lambertian>(color{0.7, 0.3, 0.3});
    auto material_left = make_shared<metal>(color{0.8, 0.8, 0.0}, 0.3);
    auto material_right = make_shared<metal>(color{0.8, 0.6, 0.2}, 1);

    world.add(make_shared<sphere>(point3(0.0, -100.5, -1.0), 100, material_ground));
    world.add(make_shared<sphere>(point3(0.0, 0.0, -1.0), 0.5, material_center));
    world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.5, material_left));
    world.add(make_shared<sphere>(point3(1.0, 0.0, -1.0), 0.5, material_right));

    // Camera
    camera cam;

    // Render    
    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    auto startTime = std::chrono::high_resolution_clock::now();


    for (int j = image_height-1; j >= 0; --j) {
        std::cout << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < image_width; ++i) {
            color pixel_color(0, 0, 0);
            for(int s = 0; s < samples_per_pixel; ++s) {
                auto u = (i + random_double()) / (image_width - 1);
                auto v = (j + random_double()) / (image_height - 1);
                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, world, max_depth);
            }

             // Divide the color by the number of samples and gamma-correct for gamma=2.0.
            pixel_color = sqrt(pixel_color / samples_per_pixel);

            image.set(i, j, pixel_color );
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<float, std::chrono::seconds::period>(endTime - startTime).count();
    std::cout << "Image generated in " << duration << "seconds\n";

    // image.write("../../results/sphereTrueLambertian.jpg");
    std::string result_path("results/sphereMaterialFuzz.jpg");
    std::cout << "\nWriting result to :: " << std::filesystem::current_path().append(result_path) << std::endl;
    if(image.write(result_path) != 0) {
        std::cout << "Success!";
    } else {
        std::cout << "Failed!";
    }
    
}