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
inline Layout computeLayout(float x, float y, float w, float h, float sidebarW = 420.0f) {
  const float side = std::min(sidebarW, w / 3.0f);
  return {{x, y, side, h}, {x + side, y, w - side, h}};
}

struct WindowSize {
  int w, h;
};

// Opening size: 80% of the monitor work area, at least 640x480 but never larger than the work area itself.
// Without monitor information (work area <= 0) fall back to the configured size.
inline WindowSize initialWindowSize(int workW, int workH, int fallbackW, int fallbackH) {
  if (workW <= 0 || workH <= 0) return {fallbackW, fallbackH};
  const int w = std::min(workW, std::max(640, static_cast<int>(workW * 0.8f)));
  const int h = std::min(workH, std::max(480, static_cast<int>(workH * 0.8f)));
  return {w, h};
}

}  // namespace glimpse::gui
