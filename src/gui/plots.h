
#include <vector>

#include "core/common.h"
#include "imgui/implot.h"

///

// Assume you have a probability density function (PDF) defined as a function
float pdf(void*, int x) {
  // Your PDF implementation here
  return static_cast<float>(std::exp(-x * x));
  //   return std::sqr(-x * x);
}

// double linear_icd(double d) { return 2 * d; }
// double linear_pdf(double d) { return 0.5; }

double linear_icd(double d) { return std::sqrt(4.0 * d); }
double linear_pdf(double d) { return d / 2.0; }

struct Funcs {
  static float Sin(void*, int i) { return sinf(i * 0.1f); }
  static float Saw(void*, int i) { return (i & 1) ? 1.0f : -1.0f; }
};

void plotPdf() {
  //   ImGui::ShowDemoWindow();
  //   ImPlot::ShowDemoWindow();
  //   return;

  static bool animate = true;

  ImGui::Begin("PDF");

  // Visualize our random number distribution
  static std::vector<float> random_doubles;
  static std::vector<float> linear_pdf_first;
  if (ImGui::Button("Sim")) {
    // samples.clear();
    for (int i = 0; i < 10000; i++) {
      auto sample = random_double();
      random_doubles.push_back(static_cast<float>(sample));
      linear_pdf_first.push_back(static_cast<float>(linear_icd(sample)));
    }
    std::cout << "Sample count :: " << random_doubles.size() << std::endl;
  }
  ImGui::PlotHistogram("Random Doubles", random_doubles.data(), int(random_doubles.size()), 0, nullptr, 0, 1, {0, 100});
  ImGui::PlotHistogram("Linear", linear_pdf_first.data(), int(linear_pdf_first.size()), 0, nullptr, 0, 2, {0, 100});

  ImGui::End();
}

void estimatePi() {
  ImGui::Begin("Estimating Pi");
  static int inside_circle = 0;
  static int inside_circle_stratified = 0;
  static int sample_count = 0;

  int sqrt_N = 100;

  for (int i = 0; i < sqrt_N; i++) {
    for (int j = 0; j < sqrt_N; j++) {
      auto x = random_double(-1, 1);
      auto y = random_double(-1, 1);
      if (x * x + y * y < 1) inside_circle++;

      x = 2 * ((i + random_double()) / sqrt_N) - 1;
      y = 2 * ((j + random_double()) / sqrt_N) - 1;
      if (x * x + y * y < 1) inside_circle_stratified++;
    }
  }
  sample_count += sqrt_N * sqrt_N;

  ImGui::Text("Regular estimate of pi :: %f", (4.0 * inside_circle) / sample_count);
  ImGui::Text("Stratified estimate of pi :: %f", (4.0 * inside_circle_stratified) / sample_count);

  ImGui::End();
}
