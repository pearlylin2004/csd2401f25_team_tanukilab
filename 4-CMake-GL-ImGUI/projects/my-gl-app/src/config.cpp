#include "config.h"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <cctype>

namespace {
    inline std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        size_t end = s.find_last_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        return s.substr(start, end - start + 1);
    }

    std::unordered_map<std::string, std::string> LoadTxtMap(const std::string& path) {
        std::unordered_map<std::string, std::string> map;
        std::ifstream in(path);
        std::string line;
        while (std::getline(in, line)) {
            if (line.empty() || line[0] == '#') continue; // skip comments
            auto eq = line.find('=');
            if (eq == std::string::npos) continue;
            std::string key = trim(line.substr(0, eq));
            std::string val = trim(line.substr(eq + 1));
            map[key] = val;
        }
        return map;
    }
}

bool LoadConfig(const std::string& path, AppConfig& out, std::string* err) {
    std::ifstream test(path);
    if (!test.is_open()) {
        if (err) *err = "Could not open " + path + " (using defaults)";
        return false;
    }
    test.close();

    auto cfg = LoadTxtMap(path);

    if (cfg.count("width"))  out.width = std::stoi(cfg["width"]);
    if (cfg.count("height")) out.height = std::stoi(cfg["height"]);
    if (cfg.count("title"))  out.title = cfg["title"];
    if (cfg.count("vsync"))  out.vsync = (cfg["vsync"] != "0");
    

    if (cfg.count("msaa_samples")) out.msaa_samples = std::stoi(cfg["msaa_samples"]);

    if (cfg.count("clear_color")) {
        std::istringstream iss(cfg["clear_color"]);
        std::string token;
        int i = 0;
        while (std::getline(iss, token, ',') && i < 4) {
            out.clear_color[i++] = std::stof(token);
        }
    }

    if (cfg.count("show_input_debug")) out.show_input_debug = (cfg["show_input_debug"] != "0");

    return true;
}
