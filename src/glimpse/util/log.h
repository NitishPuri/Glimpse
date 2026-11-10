#pragma once

// Logging on top of std::format, as free functions like Glint's (core/log.h):
//   glimpse::log::info("Rendered {} at {}x{}", name, width, height);
// Every line goes to the console (warn/error to stderr) and, once log::setFile has opened one, to that file too.
// Thread-safe: the render threads and the viewer's UI thread log at the same time. Console lines carry seconds
// since start; file lines also carry the wall-clock time. logFileFor(app) names a per-run file.

#include <chrono>
#include <format>
#include <string>
#include <string_view>

namespace glimpse {

namespace log {

enum class Level { Debug, Info, Warn, Error };

void write(Level level, std::string_view message);

// Also write every message to this file (truncated on open; missing directories are created). Empty path closes
// it. Returns false if the file can't be opened.
bool setFile(const std::string& path);

// Console output on/off (default on); the file, if open, still gets every line. Tests use it to stay quiet.
void setConsole(bool enabled);

// Messages below this level are dropped. Default: Debug in debug builds, Info otherwise.
void setMinLevel(Level level);

// The format string is checked at compile time against the argument types.
template <typename... Args>
void debug(std::format_string<Args...> f, Args&&... args) {
  write(Level::Debug, std::format(f, std::forward<Args>(args)...));
}
template <typename... Args>
void info(std::format_string<Args...> f, Args&&... args) {
  write(Level::Info, std::format(f, std::forward<Args>(args)...));
}
template <typename... Args>
void warn(std::format_string<Args...> f, Args&&... args) {
  write(Level::Warn, std::format(f, std::forward<Args>(args)...));
}
template <typename... Args>
void error(std::format_string<Args...> f, Args&&... args) {
  write(Level::Error, std::format(f, std::forward<Args>(args)...));
}

}  // namespace log

// The local wall-clock time now, truncated to Precision, for std::format's chrono specs ("{:%Y%m%d_%H%M%S}").
// The truncation matters: %S prints fractional seconds down to the time point's precision.
template <typename Precision = std::chrono::seconds>
auto localNow() {
  const auto now = std::chrono::floor<Precision>(std::chrono::system_clock::now());
  return std::chrono::zoned_time{std::chrono::current_zone(), now};
}

// One log file per run, like Glint: logs/<app>_<YYYYmmdd_HHMMSS>.log (relative to the repo root, where run.sh runs).
inline std::string logFileFor(const std::string& app) {
  return std::format("logs/{}_{:%Y%m%d_%H%M%S}.log", app, localNow());
}

}  // namespace glimpse
