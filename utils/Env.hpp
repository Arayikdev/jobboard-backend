#pragma once
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class Env {
public:
  static void load(const std::string &filename = ".env") {
    std::vector<std::string> searchPaths = {filename, "../" + filename,
                                            "../../" + filename};

    std::ifstream file;
    std::string loadedPath;

    for (const auto &path : searchPaths) {
      file.open(path);
      if (file.is_open()) {
        loadedPath = path;
        break;
      }
    }

    if (!file.is_open()) {
      std::cerr << "Warning: Could not find " << filename
                << " in current or parent directories." << std::endl;
      return;
    }

    std::cout << "Loaded environment from: " << loadedPath << std::endl;

    std::string line;
    while (std::getline(file, line)) {
      // Skip comments and empty lines
      if (line.empty() || line[0] == '#')
        continue;

      auto delimiterPos = line.find('=');
      if (delimiterPos != std::string::npos) {
        std::string key = line.substr(0, delimiterPos);
        std::string value = line.substr(delimiterPos + 1);

        // Remove carriage return if present (handle Windows-style line endings)
        if (!value.empty() && value.back() == '\r') {
          value.pop_back();
        }

        // Set environment variable
        setenv(key.c_str(), value.c_str(), 1);
      }
    }
  }

  static std::string get(const std::string &key,
                         const std::string &defaultValue = "") {
    const char *val = std::getenv(key.c_str());
    if (val == nullptr)
      return defaultValue;
    return std::string(val);
  }
};
