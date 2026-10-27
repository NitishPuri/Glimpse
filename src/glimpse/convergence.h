#pragma once

// Convergence measurement (technique 02): render a scene at increasing spp, time each render, and measure its error
// against the reference. Error vs spp shows the estimator's rate: an unbiased Monte Carlo estimator's MSE is its
// variance, which falls as 1/N, a slope of -1 on log-log axes; a flatter tail means bias (or a bad reference).
// Error vs seconds is what an equal-time comparison reads off: pbrt-v4 §2.1.3's efficiency is 1 / (variance * time).
// Each point is an independent render, not a snapshot of one progressive render, because the stratified grid
// (sqrtSpp x sqrtSpp) is fixed per render; it also means each time includes the BVH build, like a real run.

#include <functional>
#include <string>
#include <vector>

#include "glimpse/util/metrics.h"

namespace glimpse {

struct FloatImage;
struct Scene;

// Default largest spp for --convergence: a 32x32 grid. Total cost is about twice that one render.
constexpr int kConvergenceMaxSpp = 1024;

// Perfect squares (the stratified grid needs one) from 1 up to maxSpp, about doubling each step: sqrtSpp grows by
// sqrt(2), so the points are evenly spaced on a log axis. Always ends at floor(sqrt(maxSpp))^2.
std::vector<int> convergenceSchedule(int maxSpp);

struct ConvergencePoint {
  int spp = 0;
  double seconds = 0;  // wall time of the whole render
  ErrorStats error;
};

// Renders `scene` once per spp in `schedule` and compares each film against `ref`. The camera must already match
// the reference's size. `onPoint` (optional) is called after each render, for progress output.
std::vector<ConvergencePoint> measureConvergence(const Scene& scene, const FloatImage& ref,
                                                 const std::vector<int>& schedule,
                                                 const std::function<void(const ConvergencePoint&)>& onPoint = {});

// One row per point: spp,seconds,mse,rmse,relmse (header included). False if the file can't be written.
bool writeConvergenceCSV(const std::string& path, const std::vector<ConvergencePoint>& points);

}  // namespace glimpse
