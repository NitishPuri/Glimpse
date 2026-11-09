#pragma once

// Logger: timestamped lines to the console and a file, on top of std::format:
//   logger.log("Rendered {} at {}x{}", name, width, height);
// logFileFor(app) names a per-run file logs/<app>_<time>.log.

#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <print>
#include <string>

#define CONSOLE_LOG 1

namespace glimpse {

// The local wall-clock time now, truncated to Precision, for std::format's chrono specs ("{:%Y%m%d_%H%M%S}").
// The truncation matters: %S prints fractional seconds down to the time point's precision.
template <typename Precision = std::chrono::seconds>
auto localNow() {
  const auto now = std::chrono::floor<Precision>(std::chrono::system_clock::now());
  return std::chrono::zoned_time{std::chrono::current_zone(), now};
}

class Logger {
 public:
  Logger(const std::string &logFilePath) : logFilePath(logFilePath) {
    const auto dir = std::filesystem::path(logFilePath).parent_path();
    if (!dir.empty()) std::filesystem::create_directories(dir);
    logFile.open(logFilePath, std::ios_base::app);  // Open in append mode
    if (!logFile.is_open()) std::println(stderr, "Unable to open log file: {}", logFilePath);
  }

  // The format string is checked at compile time against the argument types.
  template <typename... Args>
  void log(std::format_string<Args...> fmt, Args &&...args) {
    const std::string line = std::format("{:%Y-%m-%d %H:%M:%S}: {}", localNow<std::chrono::milliseconds>(),
                                         std::format(fmt, std::forward<Args>(args)...));
#if CONSOLE_LOG
    std::println(stderr, "{}", line);
#endif
    if (logFile.is_open()) {
      std::println(logFile, "{}", line);
      logFile.flush();  // keep the file useful even if the run crashes right after
    }
  }

 private:
  std::string logFilePath;
  std::ofstream logFile;
};

// One log file per run, like Glint: logs/<app>_<YYYYmmdd_HHMMSS>.log (relative to the repo root, where run.sh runs).
inline std::string logFileFor(const std::string &app) {
  return std::format("logs/{}_{:%Y%m%d_%H%M%S}.log", app, localNow());
}

}  // namespace glimpse
