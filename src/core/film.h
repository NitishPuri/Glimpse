#pragma once

#include <vector>

#include "vec3.h"

class Film {
 public:
  Film() : m_Width(0), m_Height(0) {}
  Film(int width, int height)
      : m_Width(width),
        m_Height(height),
        sample_count(width * height, 0),
        accumulated_samples(width * height, vec3(0, 0, 0)),
        mean(width * height, vec3(0, 0, 0)),
        m2(width * height, vec3(0, 0, 0)) {}

  void initialize(int width, int height) {
    this->m_Width = width;
    this->m_Height = height;
    sample_count.resize(width * height, 0);
    accumulated_samples.resize(width * height, vec3(0, 0, 0));
    mean.resize(width * height, vec3(0, 0, 0));
    m2.resize(width * height, vec3(0, 0, 0));
  }

  void add_sample(int x, int y, const vec3& sample) {
    if (!isValid(x, y)) return;
    // look for and remove NaNs

    int index = get_index(x, y);
    sample_count[index]++;

    // TODO: Enable variance calcualtion after initial setup.
    // TODO: Actually use variance map to do adaptive sampling!
    //  vec3 delta = sample - mean[index];
    //  mean[index] += delta / sample_count[index];
    //  vec3 delta2 = sample - mean[index];
    //  m2[index] += delta * delta2;

    accumulated_samples[index] += sample;
  }

  vec3 get_mean(int x, int y) const {
    if (!isValid(x, y)) return {};
    int index = get_index(x, y);
    return mean[index];
  }

  vec3 get_variance(int x, int y) const {
    if (!isValid(x, y)) return {};
    int index = get_index(x, y);
    if (sample_count[index] < 2) {
      return vec3(0, 0, 0);
    }
    return m2[index] / (sample_count[index] - 1);
  }

  vec3 get_accumulated_sample(int x, int y) const {
    if (!isValid(x, y)) return {};
    int index = get_index(x, y);
    return accumulated_samples[index];
  }

  int get_sample_count(int x, int y) const {
    if (!isValid(x, y)) return {};
    int index = get_index(x, y);
    return sample_count[index];
  }

  int get_average_sample_count() const {
    int total = 0;
    for (int i = 0; i < sample_count.size(); i++) {
      total += sample_count[i];
    }
    return int(total / sample_count.size());
  }

  bool isValid(int x, int y) const { return x >= 0 && x < m_Width && y >= 0 && y < m_Height; }

  vec3 get_sample(int x, int y) const {
    if (!isValid(x, y)) return {};
    int index = get_index(x, y);
    if (sample_count[index] == 0) {
      return vec3(0, 0, 0);
    }
    return accumulated_samples[index] / sample_count[index];
  }

  inline int get_index(int x, int y) const { return y * m_Width + x; }

 private:
  int m_Width, m_Height;
  std::vector<int> sample_count;
  std::vector<vec3> accumulated_samples;
  std::vector<vec3> mean;
  std::vector<vec3> m2;
};