#include "glimpse/util/metrics.h"

#include <cmath>
#include <limits>

#include "glimpse/util/float_image.h"
#include "glimpse/util/image.h"

//
#include "../test_cfg.h"

using namespace glimpse;

namespace {
FloatImage filled(int w, int h, float v) {
  FloatImage img(w, h);
  for (auto& c : img.rgb) c = v;
  return img;
}
}  // namespace

static ut::suite<"metrics"> metricsSuite = [] {
  using namespace boost::ut;

  "metrics"_test = [] {
    "identical_images_have_zero_error"_test = [] {
      const FloatImage a = filled(4, 3, 0.37f);
      const auto s = compareImages(a, a);
      expect(s.has_value() >> fatal);
      expect(s->mse == 0.0_d && s->rmse == 0.0_d && s->relMSE == 0.0_d && s->nonFinite == 0_u);
    };

    // One pixel, ref (1, 1, 1), test (2, 1, 1): one channel off by 1.
    "hand_computed_single_pixel"_test = [] {
      FloatImage ref = filled(1, 1, 1.0f), test = filled(1, 1, 1.0f);
      test.rgb[0] = 2.0f;
      const auto s = compareImages(test, ref);
      expect(s.has_value() >> fatal);
      expect(std::abs(s->mse - 1.0 / 3.0) < 1e-12) << "mean over all channels";
      expect(std::abs(s->rmse - std::sqrt(1.0 / 3.0)) < 1e-12);
      expect(std::abs(s->relMSE - (1.0 / (1.01 * 1.01)) / 3.0) < 1e-12) << "pbrt MRSE: (v-ref)^2 / (ref+0.01)^2";
    };

    // The point of relMSE: the same 10% error counts (almost) the same in a dark and a bright region, while MSE is
    // dominated by the bright one.
    "relMSE_weights_dark_and_bright_alike"_test = [] {
      const FloatImage darkRef = filled(1, 1, 0.1f), darkTest = filled(1, 1, 0.11f);
      const FloatImage brightRef = filled(1, 1, 10.0f), brightTest = filled(1, 1, 11.0f);
      const auto d = compareImages(darkTest, darkRef), b = compareImages(brightTest, brightRef);
      expect(d.has_value() && b.has_value());
      expect(b->mse / d->mse > 9000.0) << "MSE: 100^2 times larger in the bright region";
      expect(std::abs(b->relMSE / d->relMSE - 1.0) < 0.25) << "relMSE: about equal (the 0.01 epsilon shifts it)";
    };

    "size_mismatch_is_rejected"_test = [] {
      expect(!compareImages(filled(2, 2, 1.0f), filled(2, 3, 1.0f)).has_value());
    };

    // NaN/inf (a broken sample) are skipped, like pbrt, and counted, so they can't hide.
    "non_finite_values_are_skipped_and_counted"_test = [] {
      FloatImage ref = filled(2, 1, 1.0f), test = filled(2, 1, 1.0f);
      test.rgb[0] = std::numeric_limits<float>::quiet_NaN();
      test.rgb[4] = std::numeric_limits<float>::infinity();
      const auto s = compareImages(test, ref);
      expect(s.has_value() >> fatal);
      expect(s->nonFinite == 2_u);
      expect(s->mse == 0.0_d) << "the remaining channels are exact";
    };

    "false_color_error_image"_test = [] {
      const FloatImage ref = filled(2, 1, 1.0f);
      FloatImage test = ref;
      test.rgb[3] = test.rgb[4] = test.rgb[5] = 3.0f;  // pixel (1, 0): relative error far above 1
      const Image e = falseColorError(test, ref);
      expect(e.width == 2_i && e.height == 1_i);
      const ImageColor exact = e.get(0, 0), wrong = e.get(1, 0);
      expect(exact.rgb[0] == 0_i && exact.rgb[1] == 0_i && exact.rgb[2] == 0_i) << "no error = black";
      expect(wrong.rgb[0] == 255_i && wrong.rgb[1] == 255_i) << "error at or above the top of the scale = white/yellow";
    };
  };
};
