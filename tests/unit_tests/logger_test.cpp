#include <filesystem>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

#include "glimpse/util/log.h"

//
#include "../test_cfg.h"

using namespace glimpse;

namespace {
std::vector<std::string> readLines(const std::filesystem::path& path) {
  std::ifstream in(path);
  std::vector<std::string> lines;
  for (std::string line; std::getline(in, line);) lines.push_back(line);
  return lines;
}
}  // namespace

static ut::suite<"logger"> loggerSuite = [] {
  using namespace boost::ut;
  namespace fs = std::filesystem;

  "logger"_test = [] {
    "log_file_for_names_a_per_run_file_in_logs"_test = [] {
      const std::string path = logFileFor("cli");
      expect(path.starts_with("logs/cli_")) << path;  // logs/cli_YYYYmmdd_HHMMSS.log
      expect(path.size() == std::string("logs/cli_20261011_123456.log").size()) << path;
      expect(path.ends_with(".log")) << path;
    };

    "set_file_creates_missing_directories"_test = [] {
      const fs::path dir = fs::path(testOutputPath("logger_test")) / "nested";
      fs::remove_all(dir.parent_path());
      expect(log::setFile((dir / "run.log").string()) >> fatal);
      log::info("hello {}", 42);
      log::setFile("");  // close, so the file is complete before reading it
      const auto lines = readLines(dir / "run.log");
      expect((lines.size() == 1_ul) >> fatal);
      expect(lines[0].contains("info  hello 42")) << lines[0];
    };

    "drops_messages_below_the_min_level"_test = [] {
      const fs::path path = testOutputPath("logger_level.log");
      expect(log::setFile(path.string()) >> fatal);
      log::setMinLevel(log::Level::Warn);
      log::info("dropped");
      log::warn("kept");
      log::setMinLevel(log::Level::Debug);
      log::setFile("");
      const auto lines = readLines(path);
      expect((lines.size() == 1_ul) >> fatal);
      expect(lines[0].contains("WARN  kept")) << lines[0];
    };

    // The render threads log while the viewer's UI thread does: every line must come out whole.
    "lines_from_threads_never_interleave"_test = [] {
      const fs::path path = testOutputPath("logger_threads.log");
      expect(log::setFile(path.string()) >> fatal);
      log::setConsole(false);  // 1600 lines are for the file, not the test output
      constexpr size_t kThreads = 8, kLines = 200;
      {
        std::vector<std::jthread> threads;
        threads.reserve(kThreads);
        for (size_t t = 0; t < kThreads; ++t)
          threads.emplace_back([t] {
            for (size_t i = 0; i < kLines; ++i) log::warn("thread {} line {} end", t, i);
          });
      }  // jthreads join here
      log::setConsole(true);
      log::setFile("");
      const auto lines = readLines(path);
      expect(lines.size() == kThreads * kLines) << lines.size();
      for (const std::string& line : lines) expect(line.ends_with(" end")) << line;
    };
  };
};
