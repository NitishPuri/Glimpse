#pragma once

// Film: per-pixel sample counts and running sums; getSample() is the mean so far (progressive rendering).
// mean/m2 are reserved for Welford's running variance (technique 05, adaptive sampling). pbrt-v4 §5.4.

#include <numeric>
#include <vector>

#include "glimpse/util/vecmath.h"

namespace glimpse {

class Film {
 public:
  Film() : width(0), height(0) {}
  Film(int width, int height)
      : width(width),
        height(height),
        sampleCount(width * height, 0),
        accumulatedSamples(width * height, Vec3(0, 0, 0)),
        mean(width * height, Vec3(0, 0, 0)),
        m2(width * height, Vec3(0, 0, 0)) {}

  void initialize(int width, int height) {
    this->width = width;
    this->height = height;
    sampleCount.resize(width * height, 0);
    accumulatedSamples.resize(width * height, Vec3(0, 0, 0));
    mean.resize(width * height, Vec3(0, 0, 0));
    m2.resize(width * height, Vec3(0, 0, 0));
  }

  void addSample(int x, int y, const Vec3& sample) {
    if (!isValid(x, y)) return;
    // look for and remove NaNs

    int index = getIndex(x, y);
    sampleCount[index]++;

    // mean/m2 (Welford's running variance) are filled in by technique 05, adaptive sampling.
    accumulatedSamples[index] += sample;
  }

  Vec3 getMean(int x, int y) const {
    if (!isValid(x, y)) return {};
    int index = getIndex(x, y);
    return mean[index];
  }

  Vec3 getVariance(int x, int y) const {
    if (!isValid(x, y)) return {};
    int index = getIndex(x, y);
    if (sampleCount[index] < 2) {
      return Vec3(0, 0, 0);
    }
    return m2[index] / (sampleCount[index] - 1);
  }

  Vec3 getAccumulatedSample(int x, int y) const {
    if (!isValid(x, y)) return {};
    int index = getIndex(x, y);
    return accumulatedSamples[index];
  }

  int getSampleCount(int x, int y) const {
    if (!isValid(x, y)) return {};
    int index = getIndex(x, y);
    return sampleCount[index];
  }

  int getAverageSampleCount() const {
    return int(std::accumulate(sampleCount.begin(), sampleCount.end(), 0) / sampleCount.size());
  }

  bool isValid(int x, int y) const { return x >= 0 && x < width && y >= 0 && y < height; }

  Vec3 getSample(int x, int y) const {
    if (!isValid(x, y)) return {};
    int index = getIndex(x, y);
    if (sampleCount[index] == 0) {
      return Vec3(0, 0, 0);
    }
    return accumulatedSamples[index] / sampleCount[index];
  }

  inline int getIndex(int x, int y) const { return y * width + x; }

 private:
  int width, height;
  std::vector<int> sampleCount;
  std::vector<Vec3> accumulatedSamples;
  std::vector<Vec3> mean;
  std::vector<Vec3> m2;
};

}  // namespace glimpse