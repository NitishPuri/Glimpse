#include "glimpse/util/log.h"

#include <atomic>
#include <cstdio>
#include <filesystem>
#include <mutex>
#include <print>
#include <utility>

namespace glimpse::log {

namespace {

// One lock for the console and the file: a line is formatted first, then written whole, so lines from different
// threads never interleave.
std::mutex g_mutex;
std::FILE* g_file = nullptr;
bool g_console = true;
// Atomic, not under the lock: write() checks it first so dropped messages cost no locking.
#ifdef NDEBUG
std::atomic<Level> g_minLevel = Level::Info;
#else
std::atomic<Level> g_minLevel = Level::Debug;
#endif

const char* levelTag(Level level) {
  switch (level) {
    case Level::Debug:
      return "debug";
    case Level::Info:
      return "info ";
    case Level::Warn:
      return "WARN ";
    case Level::Error:
      return "ERROR";
  }
  std::unreachable();  // the switch covers every Level; -Wswitch flags a new one
}

// Time since the first log line: short, and handy for spotting slow steps.
double secondsSinceStart() {
  static const auto start = std::chrono::steady_clock::now();
  return std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
}

}  // namespace

void write(Level level, std::string_view message) {
  if (level < g_minLevel) return;
  const std::string line = std::format("[{:8.3f}] {} {}\n", secondsSinceStart(), levelTag(level), message);
  std::lock_guard lock(g_mutex);
  if (g_console) {
    std::FILE* console = level >= Level::Warn ? stderr : stdout;
    std::fputs(line.c_str(), console);
    std::fflush(console);  // stdout is block-buffered when redirected: don't lose lines if the run is killed
  }
  if (g_file) {
    std::print(g_file, "{:%H:%M:%S} {}", localNow<std::chrono::milliseconds>(), line);
    std::fflush(g_file);  // keep the file useful even if the run crashes right after
  }
}

bool setFile(const std::string& path) {
  std::lock_guard lock(g_mutex);
  if (g_file) std::fclose(g_file);
  g_file = nullptr;
  if (path.empty()) return true;
  const auto dir = std::filesystem::path(path).parent_path();
  std::error_code ec;  // a failure here shows up as fopen failing below
  if (!dir.empty()) std::filesystem::create_directories(dir, ec);
  g_file = std::fopen(path.c_str(), "w");
  if (!g_file) {
    std::println(stderr, "log: cannot open '{}'", path);
    return false;
  }
  return true;
}

void setConsole(bool enabled) {
  std::lock_guard lock(g_mutex);
  g_console = enabled;
}

void setMinLevel(Level level) { g_minLevel = level; }

}  // namespace glimpse::log
