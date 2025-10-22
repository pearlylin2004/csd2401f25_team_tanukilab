/* Start Header ************************************************************************/
/*!
\file		config.cpp
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date		October, 1st, 2025
\brief      Implementation of configuration loading for the application. 

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#include "config.h"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <cctype>

// -----------------------------------------------------------------------------
// Global configuration variables (default values)
// -----------------------------------------------------------------------------

bool  gShowFpsInTitle = true;  //< Whether FPS should be displayed in the window title              
bool  gShowInputDebug = false; // Whether input debug messages should be shown
float gClearColor[4] = { 0.2f,0.2f,0.25f,1.0f }; // Default background clear color (RGBA)

namespace {
 /**
 * @brief Trim whitespace from both ends of a string.
 *
 * @param s Input string.
 * @return Trimmed string.
 */
    inline std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        size_t end = s.find_last_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        return s.substr(start, end - start + 1);
    }

 /**
 * @brief Load a text file into a key-value map.
 *
 * Each line must follow the format `key = value`.
 * Lines starting with `#` are treated as comments and skipped.
 *
 * @param path File path to configuration file.
 * @return std::unordered_map of key-value pairs.
 */
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
/**
 * @brief Load application configuration from a file.
 *
 * Parses a configuration file into an AppConfig structure.
 * Falls back to defaults if the file cannot be opened.
 *
 * @param path Path to the configuration file.
 * @param out Reference to AppConfig to populate.
 * @param err Optional error message output (set if file cannot be opened).
 * @return true if the file was successfully loaded and parsed, false otherwise.
 */
bool LoadConfig(const std::string& path, AppConfig& out, std::string* err) {
    std::ifstream test(path);
    if (!test.is_open()) {
        if (err) *err = "Could not open " + path + " (using defaults)";
        return false;// failed, leave defaults untouched
    }
    test.close();

    auto cfg = LoadTxtMap(path);
    // Apply known configuration keys
    if (cfg.count("width"))  out.width = std::stoi(cfg["width"]);
    if (cfg.count("height")) out.height = std::stoi(cfg["height"]);
    if (cfg.count("title"))  out.title = cfg["title"];
    if (cfg.count("vsync"))  out.vsync = (cfg["vsync"] != "0");
    

    // Parse RGBA clear color (comma-separated)
    if (cfg.count("clear_color")) {
        std::istringstream iss(cfg["clear_color"]);
        std::string token;
        int i = 0;
        while (std::getline(iss, token, ',') && i < 4) {
            out.clear_color[i++] = std::stof(token);
        }
    }
    // Parse debug/display flags
    if (cfg.count("show_fps_in_title")) out.show_fps_in_title = (cfg["show_fps_in_title"] != "0");
    if (cfg.count("show_input_debug")) out.show_input_debug = (cfg["show_input_debug"] != "0");

    return true;
}
