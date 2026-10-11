#pragma once

// Logger: timestamped lines to the console and a file; logFileFor(app) names a per-run file logs/<app>_<time>.log.

#ifdef _MSC_VER
#pragma warning(disable : 4996)  // MSVC flags std::localtime as unsafe
#endif

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#define CONSOLE_LOG 1

namespace glimpse {


class Logger {
 public:
  Logger(const std::string &logFilePath) : logFilePath(logFilePath) {
    const auto dir = std::filesystem::path(logFilePath).parent_path();
    if (!dir.empty()) std::filesystem::create_directories(dir);
    logFile.open(logFilePath, std::ios_base::app);  // Open in append mode
    if (!logFile.is_open()) {
      std::cerr << "Unable to open log file: " << logFilePath << std::endl;
    }
  }

  ~Logger() {
    if (logFile.is_open()) {
      logFile.close();
    }
  }

  template <typename... Args>
  void log(Args... args) {
    std::ostringstream oss;
    oss << getCurrentTimestamp() << ": ";
    (oss << ... << args);  // Fold expression to handle variadic arguments

    std::string timestampedMessage = oss.str();
#if CONSOLE_LOG
    // Log to console
    std::clog << timestampedMessage << std::endl;
#endif

    // Log to file
    if (logFile.is_open()) {
      logFile << timestampedMessage << std::endl;
    }
  }

 private:
  std::string logFilePath;
  std::ofstream logFile;

  std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto inTimeT = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream ss;
    ss << std::put_time(std::localtime(&inTimeT), "%Y-%m-%d-%H-%M-%S") << '-' << std::setfill('0') << std::setw(3)
       << ms.count();
    return ss.str();
  }
};

// One log file per run, like Glint: logs/<app>_<YYYYmmdd_HHMMSS>.log (relative to the repo root, where run.sh runs).
inline std::string logFileFor(const std::string &app) {
  const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::ostringstream ss;
  ss << "logs/" << app << '_' << std::put_time(std::localtime(&now), "%Y%m%d_%H%M%S") << ".log";
  return ss.str();
}

}  // namespace glimpse
