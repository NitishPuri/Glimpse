#include "glimpse/util/logger.h"

#include <filesystem>
#include <fstream>
#include <string>

//
#include "../test_cfg.h"

void logger_test() {
  using namespace boost::ut;
  namespace fs = std::filesystem;

  "logger"_test = [] {
    "log_file_for_names_a_per_run_file_in_logs"_test = [] {
      const std::string path = log_file_for("cli");
      expect(path.rfind("logs/cli_", 0) == 0) << path;          // logs/cli_YYYYmmdd_HHMMSS.log
      expect(path.size() == std::string("logs/cli_20261011_123456.log").size()) << path;
      expect(path.substr(path.size() - 4) == ".log") << path;
    };

    "creates_missing_directories"_test = [] {
      const fs::path dir = fs::path(test_output_path("logger_test")) / "nested";
      fs::remove_all(dir.parent_path());
      {
        Logger logger((dir / "run.log").string());
        logger.log("hello ", 42);
      }
      std::ifstream in(dir / "run.log");
      std::string line;
      std::getline(in, line);
      expect(line.find("hello 42") != std::string::npos) << line;
    };
  };
}
