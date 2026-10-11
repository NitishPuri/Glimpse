#include "glimpse/integrators.h"

#include "glimpse/materials.h"
#include "glimpse/pdf.h"

namespace glimpse {

// Recursive ray tracing with depth limiting
color ray_color(const ray &r, const color &background, const hittable &world, int depth, const hittable &lights,
                bool has_lights) {
  hit_record rec;

  // if we've exceeded the ray bounce limit, no more light is gathered
  if (depth < 0) return color(0, 0, 0);

  // If the ray hits nothing, return the background color.
  // Use eps = 0.001 to avoid self-intersections
  if (world.hit(r, interval{0.001, math::infinity}, rec)) {
    scatter_record srec;
    color color_from_emission = rec.mat->emitted(r, rec, rec.u, rec.v, rec.p);

    // Scattered reflectance
    if (rec.mat->scatter(r, rec, srec)) {
      if (srec.skip_pdf) {
        return srec.attenuation * ray_color(srec.skip_pdf_ray, background, world, depth - 1, lights, has_lights);
      }

      ray scattered;
      double pdf_value;
      if (has_lights) {
        auto light_ptr = make_shared<hittable_pdf>(lights, rec.p);
        mixture_pdf mixed_pdf(light_ptr, srec.pdf_ptr);
        scattered = ray(rec.p, mixed_pdf.generate(), r.time());
        pdf_value = mixed_pdf.value(scattered.direction());
      } else {
        scattered = ray(rec.p, srec.pdf_ptr->generate(), r.time());
        pdf_value = srec.pdf_ptr->value(scattered.direction());
      }

      double scattering_pdf = rec.mat->scattering_pdf(r, rec, scattered);

      color sample_color = ray_color(scattered, background, world, depth - 1, lights, has_lights);

      color color_from_scatter = (srec.attenuation * scattering_pdf * sample_color) / pdf_value;

      return color_from_emission + color_from_scatter;
    }

    return color_from_emission;
  }

  return background;
  // vec3 unit_direction = unit_vector(r.direction());
  // auto t = 0.5 * (unit_direction.y() + 1.0);
  // return (1.0 - t) * color(1.0, 1.0, 1.0) + t * background;
}

}  // namespace glimpse
