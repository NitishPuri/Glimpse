#include "glimpse/util/color.h"

#include <cmath>

#include "glimpse/util/float_image.h"
#include "glimpse/util/image.h"

//
#include "../test_cfg.h"

using namespace glimpse;

static ut::suite<"color"> colorSuite = [] {
  using namespace boost::ut;

  "srgb"_test = [] {
    // Reference values from the sRGB definition (IEC 61966-2-1), computed independently.
    "transfer_function_values"_test = [] {
      expect(linearToSRGB(0.0) == 0.0_d);
      expect(std::abs(linearToSRGB(0.0031308) - 0.040449936) < 1e-6) << "the linear toe meets the power curve";
      expect(std::abs(linearToSRGB(0.18) - 0.46135613) < 1e-6) << "18% grey";
      expect(std::abs(linearToSRGB(0.5) - 0.73535698) < 1e-6);
      expect(std::abs(linearToSRGB(1.0) - 1.0) < 1e-9);
    };

    "linear_toe_near_black"_test = [] {
      expect(std::abs(linearToSRGB(0.001) - 0.01292) < 1e-9) << "12.92 * x below 0.0031308";
    };

    "monotonic"_test = [] {
      double prev = -1;
      bool ok = true;
      for (int i = 0; i <= 1000; ++i) {
        const double v = linearToSRGB(i / 1000.0);
        ok = ok && v > prev;
        prev = v;
      }
      expect(ok);
    };

    "encode_applies_exposure_and_clamps"_test = [] {
      FloatImage lin(3, 1);
      lin.rgb = {0.18f, 0.18f, 0.18f, 0.09f, 0.09f, 0.09f, 5.0f, 5.0f, 5.0f};  // grey, half grey, over-exposed
      const Image at0 = encodeSRGB(lin, 0.0);
      const Image plus1 = encodeSRGB(lin, 1.0);  // +1 stop doubles the linear value: 0.09 -> 0.18
      expect(at0.width == 3_i && at0.height == 1_i);
      expect(at0.get(0, 0).rgb[0] == plus1.get(1, 0).rgb[0]) << "+1 stop on 0.09 equals 0 stops on 0.18";
      expect(at0.get(2, 0).rgb[0] == 255_i) << "radiance above 1 clamps to white";
      expect(at0.get(0, 0).rgb[0] == static_cast<int>(256.0f * static_cast<float>(linearToSRGB(0.18f))));
    };
  };
};
