#pragma once

#include <vector>

#include "vec3.h"

class Film {
 public:
  Film() : width(0), height(0) {}
  Film(int width, int height)
      : width(width),
        height(height),
        sample_count(width * height, 0),
        accumulated_samples(width * height, vec3(0, 0, 0)),
        mean(width * height, vec3(0, 0, 0)),
        m2(width * height, vec3(0, 0, 0)) {}

  void initialize(int width, int height) {
    this->width = width;
    this->height = height;
    sample_count.resize(width * height, 0);
    accumulated_samples.resize(width * height, vec3(0, 0, 0));
    mean.resize(width * height, vec3(0, 0, 0));
    m2.resize(width * height, vec3(0, 0, 0));
  }

  void add_sample(int x, int y, const vec3& sample) {
    // look for and remove NaNs

    int index = get_index(x, y);
    sample_count[index]++;

    // TODO: Enable variance calcualtion after initial setup.
    //  vec3 delta = sample - mean[index];
    //  mean[index] += delta / sample_count[index];
    //  vec3 delta2 = sample - mean[index];
    //  m2[index] += delta * delta2;

    accumulated_samples[index] += sample;
  }

  vec3 get_mean(int x, int y) const {
    int index = get_index(x, y);
    return mean[index];
  }

  vec3 get_variance(int x, int y) const {
    int index = get_index(x, y);
    if (sample_count[index] < 2) {
      return vec3(0, 0, 0);
    }
    return m2[index] / (sample_count[index] - 1);
  }

  vec3 get_accumulated_sample(int x, int y) const {
    int index = get_index(x, y);
    return accumulated_samples[index];
  }

  int get_sample_count(int x, int y) const {
    int index = get_index(x, y);
    return sample_count[index];
  }

  vec3 get_sample(int x, int y) const {
    int index = get_index(x, y);
    if (sample_count[index] == 0) {
      return vec3(0, 0, 0);
    }
    return accumulated_samples[index] / sample_count[index];
  }

  inline int get_index(int x, int y) const { return y * width + x; }

 private:
  int width, height;
  std::vector<int> sample_count;
  std::vector<vec3> accumulated_samples;
  std::vector<vec3> mean;
  std::vector<vec3> m2;
};