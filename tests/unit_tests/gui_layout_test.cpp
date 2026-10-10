#include "gui/layout.h"

//
#include "../test_cfg.h"

using namespace glimpse::gui;

void gui_layout_test() {
  using namespace boost::ut;

  "gui_layout"_test = [] {
    "sidebar_and_output_fill_the_window"_test = [] {
      auto l = compute_layout(0, 20, 1600, 900);  // y = 20: below a menu bar / work-area offset
      expect(l.controls.x == 0.0_f && l.controls.y == 20.0_f);
      expect(l.controls.w == 420.0_f && l.controls.h == 900.0_f);
      expect(l.output.x == 420.0_f && l.output.y == 20.0_f);
      expect(l.output.w == 1180.0_f && l.output.h == 900.0_f);
    };

    "narrow_window_caps_sidebar_at_a_third"_test = [] {
      auto l = compute_layout(0, 0, 900, 600);
      expect(l.controls.w == 300.0_f);
      expect(l.output.x == 300.0_f && l.output.w == 600.0_f);
    };

    "initial_size_is_80_percent_of_work_area"_test = [] {
      auto s = initial_window_size(5120, 1410, 1800, 1600);
      expect(s.w == 4096_i && s.h == 1128_i);
    };

    "small_screen_gets_the_640x480_minimum"_test = [] {
      auto s = initial_window_size(700, 500, 1800, 1600);  // 80% would be 560x400
      expect(s.w == 640_i && s.h == 480_i);
    };

    "minimum_never_exceeds_the_screen"_test = [] {
      auto s = initial_window_size(600, 400, 1800, 1600);
      expect(s.w == 600_i && s.h == 400_i) << "the 640x480 minimum is clamped to the work area";
    };

    "no_monitor_info_uses_fallback"_test = [] {
      auto s = initial_window_size(0, 0, 1800, 1600);
      expect(s.w == 1800_i && s.h == 1600_i);
    };
  };
}
