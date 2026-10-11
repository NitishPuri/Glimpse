// rayColor: one-sample estimate of the radiance arriving along a ray (pbrt-v4 §13.2–13.3, TROYL).
// Each bounce adds emission, then scatters: specular materials follow their one ray, others sample a direction
// from the material pdf, or a 50/50 mixture with the light pdf when the scene lists lights, and weight the result by
// BRDF * cos / pdf.

#include "glimpse/integrators.h"

#include "glimpse/materials.h"
#include "glimpse/pdf.h"

namespace glimpse {

// Recursive ray tracing with depth limiting
Color rayColor(const Ray &r, const Color &background, const Hittable &world, int depth, const Hittable &lights,
                bool hasLights) {
  HitRecord rec;

  // if we've exceeded the ray bounce limit, no more light is gathered
  if (depth < 0) return Color(0, 0, 0);

  // If the ray hits nothing, return the background color.
  // Use eps = 0.001 to avoid self-intersections
  if (world.hit(r, Interval{0.001, math::infinity}, rec)) {
    ScatterRecord srec;
    Color colorFromEmission = rec.mat->emitted(r, rec, rec.u, rec.v, rec.p);

    // Scattered reflectance
    if (rec.mat->scatter(r, rec, srec)) {
      if (srec.skipPdf) {
        return srec.attenuation * rayColor(srec.skipPdfRay, background, world, depth - 1, lights, hasLights);
      }

      Ray scattered;
      double pdfValue;
      if (hasLights) {
        auto lightPtr = make_shared<HittablePDF>(lights, rec.p);
        MixturePDF mixedPdf(lightPtr, srec.pdfPtr);
        scattered = Ray(rec.p, mixedPdf.generate(), r.time());
        pdfValue = mixedPdf.value(scattered.direction());
      } else {
        scattered = Ray(rec.p, srec.pdfPtr->generate(), r.time());
        pdfValue = srec.pdfPtr->value(scattered.direction());
      }

      double scatteringPdf = rec.mat->scatteringPdf(r, rec, scattered);

      Color sampleColor = rayColor(scattered, background, world, depth - 1, lights, hasLights);

      Color colorFromScatter = (srec.attenuation * scatteringPdf * sampleColor) / pdfValue;

      return colorFromEmission + colorFromScatter;
    }

    return colorFromEmission;
  }

  return background;
}

}  // namespace glimpse
