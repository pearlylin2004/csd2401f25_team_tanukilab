#pragma once
#include <string>

struct AppConfig {
    // window
    int         width = 1600;
    int         height = 900;
    std::string title = "Tanuki Lab";
    bool        vsync = true;
    bool        fullscreen = false;

    // graphics

    float clear_color[4] = { 0.2f, 0.2f, 0.2f, 1.0f };

    bool  show_input_debug = false;
    bool show_fps_in_title = true;
};

// Returns true if loaded successfully (defaults remain if file missing/invalid)
bool LoadConfig(const std::string& path, AppConfig& out, std::string* err = nullptr);

extern bool  gShowFpsInTitle;
extern bool  gShowInputDebug;
extern float gClearColor[4];