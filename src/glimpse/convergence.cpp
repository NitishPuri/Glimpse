#include "glimpse/convergence.h"

#include <chrono>
#include <cmath>
#include <fstream>

#include "glimpse/render.h"
#include "glimpse/scene.h"
#include "glimpse/util/float_image.h"
#include "glimpse/util/image.h"

namespace glimpse {

std::vector<int> convergenceSchedule(int maxSpp) {
  std::vector<int> spp;
  const int maxSqrt = static_cast<int>(std::sqrt(maxSpp));  // the same rounding as Camera::initialize
  int last = 0;
  for (double s = 1; std::lround(s) <= maxSqrt; s *= std::sqrt(2.0)) {
    const int q = static_cast<int>(std::lround(s));
    if (q != last) spp.push_back(q * q);  // 1 and sqrt(2) both round to 1
    last = q;
  }
  if (last != maxSqrt && maxSqrt >= 1) spp.push_back(maxSqrt * maxSqrt);
  return spp;
}

std::vector<ConvergencePoint> measureConvergence(const Scene& scene, const FloatImage& ref,
                                                 const std::vector<int>& schedule,
                                                 const std::function<void(const ConvergencePoint&)>& onPoint) {
  std::vector<ConvergencePoint> points;
  for (int spp : schedule) {
    Scene s = scene;
    s.cam.samplesPerPixel = spp;
    s.cam.initialize();
    Image image(s.cam.imageWidth, s.cam.imageHeight);
    Renderer renderer;  // fresh film per point: every render starts from zero samples

    const auto start = std::chrono::steady_clock::now();
    renderer.renderScene(s, image, nullptr);
    const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();

    const auto error = compareImages(renderer.film.linearRGB(), ref);
    if (!error) break;  // size mismatch: the caller sizes the camera from the reference, so this is a bug
    points.push_back({spp, seconds, *error});
    if (onPoint) onPoint(points.back());
  }
  return points;
}

bool writeConvergenceCSV(const std::string& path, const std::vector<ConvergencePoint>& points) {
  std::ofstream out(path);
  if (!out) return false;
  out.precision(9);
  out << "spp,seconds,mse,rmse,relmse\n";
  for (const auto& p : points)
    out << p.spp << ',' << p.seconds << ',' << p.error.mse << ',' << p.error.rmse << ',' << p.error.relMSE << '\n';
  return static_cast<bool>(out);
}

}  // namespace glimpse
