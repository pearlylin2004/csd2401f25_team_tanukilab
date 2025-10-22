/* Start Header ************************************************************************/
/*!
\file		config.h
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date		October, 1st, 2025
\brief      Declaration of application configuration structures and functions.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <cctype>
/**
 * @brief Holds application configuration values.
 *
 * Provides default window properties, graphics settings, and debug flags.
 * These can be overridden by values loaded from a configuration file.
 */
struct AppConfig {
    // window
    int         width = 1600;            ///< Default window width
    int         height = 900;            ///< Default window height
    std::string title = "Tanuki Lab";    ///< Default window title
    bool        vsync = true;            ///< Enable/disable vertical sync
    bool        fullscreen = false;      ///< Fullscreen toggle

    // graphics

    float clear_color[4] = { 0.2f, 0.2f, 0.2f, 1.0f };


    bool  show_input_debug = false;
    bool show_fps_in_title = true;
};

/**
 * @brief Load application configuration from a file.
 *
 * Reads key-value pairs from a text file and populates the provided AppConfig.
 * If the file is missing or invalid, default values remain in place.
 *
 * @param path Path to the configuration file.
 * @param out Reference to AppConfig structure to fill.
 * @param err Optional pointer to a string for error messages.
 * @return true if successfully loaded, false if file missing/invalid.
 */
bool LoadConfig(const std::string& path, AppConfig& out, std::string* err = nullptr);

extern bool  gShowFpsInTitle;
extern bool  gShowInputDebug;
extern float gClearColor[4];