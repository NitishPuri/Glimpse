#pragma once

#include <algorithm>

// Viewer layout math, free of ImGui/GL so the unit tests can cover it.
namespace glimpse::gui {

struct Rect {
  float x, y, w, h;
};

struct Layout {
  Rect controls;  // left sidebar
  Rect output;    // render view, fills the rest
};

// (x, y, w, h) is the usable area (ImGui main viewport work area). The sidebar keeps a fixed width but never
// takes more than a third of a narrow window.
inline Layout compute_layout(float x, float y, float w, float h, float sidebar_w = 420.0f) {
  const float side = std::min(sidebar_w, w / 3.0f);
  return {{x, y, side, h}, {x + side, y, w - side, h}};
}

struct WindowSize {
  int w, h;
};

// Opening size: 80% of the monitor work area, at least 640x480 but never larger than the work area itself.
// Without monitor information (work area <= 0) fall back to the configured size.
inline WindowSize initial_window_size(int work_w, int work_h, int fallback_w, int fallback_h) {
  if (work_w <= 0 || work_h <= 0) return {fallback_w, fallback_h};
  const int w = std::min(work_w, std::max(640, static_cast<int>(work_w * 0.8f)));
  const int h = std::min(work_h, std::max(480, static_cast<int>(work_h * 0.8f)));
  return {w, h};
}

}  // namespace glimpse::gui
