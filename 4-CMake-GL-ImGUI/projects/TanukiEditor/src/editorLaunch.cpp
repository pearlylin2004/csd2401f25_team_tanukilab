
//#ifdef TANUKI_EDITOR   
#include "editorLaunch.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <direct.h>

namespace fs = std::filesystem;

/*
//  Minimal stub for new.json  (empty scene so the editor can open it)
// ============================================================================ */
EditorLaunch::EditorLaunch() {}

/* -------- Layout Constant -------- */
struct LaunchLayout {
    float centerX;
    float titleY;
    float subtitleY;
    float contentY;
    ImVec2 winSize;
};
/* -------- END -------- */

static LaunchLayout computeLayout() {
    LaunchLayout L;
    L.winSize   = ImGui::GetContentRegionAvail();
    L.centerX        = L.winSize.x * 0.5f;
    L.titleY    = L.winSize.y * 0.18f;
    L.subtitleY = L.titleY + 52.0f;
    L.contentY  = L.subtitleY + 60.0f;
    return L;
}

// render background
static void renderBackground() {
    ImGuiIO&  io = ImGui::GetIO();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    const ImVec2 p0 = ImGui::GetWindowPos();
    const ImVec2 p1 = ImVec2(p0.x + io.DisplaySize.x, p0.y + io.DisplaySize.y);

    dl->AddRectFilled(p0, p1, IM_COL32(28, 28, 35, 255)); // dark charcoal
}

// render title
static void renderTitle(const LaunchLayout& L) {
    const char* title = "Tanuki Engine";

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts.Size > 1
                    ? ImGui::GetIO().Fonts->Fonts[1]
                    : ImGui::GetIO().Fonts->Fonts[0]);

    ImGui::SetCursorPos(ImVec2(L.centerX - ImGui::CalcTextSize(title).x * 0.5f, L.titleY));
    ImGui::TextColored(ImVec4(0.95f, 0.88f, 0.65f, 1.0f), "%s", title);
    ImGui::PopFont();

    const char* sub = "Project Launcher";
    ImGui::SetCursorPos(ImVec2(L.centerX - ImGui::CalcTextSize(sub).x * 0.5f, L.subtitleY));
    ImGui::TextColored(ImVec4(0.55f, 0.55f, 0.65f, 1.0f), "%s", sub);
}

// actual rendering
bool EditorLaunch::render() {
    if (m_done) return true;

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::SetNextWindowBgAlpha(1.0f);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoFocusOnAppearing;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("##TanukiLauncher", nullptr, flags);
    ImGui::PopStyleVar(3);

    const LaunchLayout L = computeLayout();

    renderBackground();
    renderTitle(L);

    switch (m_screen) {
        case Screen::HOME:
            renderHomeScreen(L);   
            break;
        case Screen::CREATE_INPUT: 
            renderCreateScreen(L); 
            break;
        case Screen::OPEN_LIST:    
            renderOpenScreen(L);   
            break;
    }

    ImGui::End();
    return m_done;
}

// render home screen (2 big big buttons, create/open)
void EditorLaunch::renderHomeScreen(const LaunchLayout& L) {
    const ImVec2 btnSize(220.0f, 60.0f);
    const float  gap    = 24.0f;
    const float  totalW = btnSize.x * 2.0f + gap;
    const float  btnY   = L.contentY + 40.0f;

    /* ---- CREATE NEW button ---- */ 
    ImGui::SetCursorPos(ImVec2(L.centerX - totalW * 0.5f, btnY));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.22f, 0.48f, 0.82f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.58f, 0.95f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.18f, 0.38f, 0.70f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

    if (ImGui::Button("  + Create New Game  ", btnSize)) {
        m_screen = Screen::CREATE_INPUT;
        m_errorMsg = "";
        memset(m_newNameBuf, 0, sizeof(m_newNameBuf));
    }

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();
    /* ---- END ---- */ 

    /* ---- OPEN EXISTING ---- */ 
    ImGui::SetCursorPos(ImVec2(L.centerX - totalW * 0.5f + btnSize.x + gap, btnY));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.42f, 0.30f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.24f, 0.55f, 0.40f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.14f, 0.33f, 0.24f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

    if (ImGui::Button("  Open Existing Game  ", btnSize)) {
        scanExistingGames();
        m_screen = Screen::OPEN_LIST;
        m_errorMsg = "";
    }

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();
    /* ---- END ---- */ 
}

// render Create New Game screen
void EditorLaunch::renderCreateScreen(const LaunchLayout& L) {
    const float fieldW = 340.f;
    const float fieldY = L.contentY + 30.f;

    // label
    ImGui::SetCursorPos(ImVec2(L.centerX - fieldW * 0.5f, fieldY));
    ImGui::TextColored(ImVec4(0.85f, 0.85f, 0.85f, 1.f), "Enter Game Name:");

    // ask for game name
    ImGui::SetCursorPos(ImVec2(L.centerX - fieldW * 0.5f, fieldY + 28.0f));
    ImGui::SetNextItemWidth(fieldW);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    ImGui::InputText("##gamename", m_newNameBuf, sizeof(m_newNameBuf));
    ImGui::PopStyleVar();

    if (!m_errorMsg.empty()) {
        ImGui::SetCursorPos(ImVec2(L.centerX - fieldW * 0.5f, fieldY + 65.0f));
        ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "%s", m_errorMsg.c_str());
    }

    const float btnRowY = fieldY + 100.0f;
    const ImVec2 btnSize(150.0f, 44.0f);

    /* ---- Create Button ---- */
    ImGui::SetCursorPos(ImVec2(L.centerX - btnSize.x - 8.0f, btnRowY));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.22f, 0.48f, 0.82f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.58f, 0.95f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.18f, 0.38f, 0.70f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

    bool doCreate = ImGui::Button("Create", btnSize);
    // can ENTER to confirm
    doCreate = doCreate || (!ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter));

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    if (doCreate)
        handleCreateConfirm();
    /* ---- END ---- */


    /* ---- Back Button ---- */
    ImGui::SetCursorPos(ImVec2(L.centerX + 8.0f, btnRowY));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.35f, 0.40f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.45f, 0.45f, 0.52f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.28f, 0.28f, 0.33f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

    if (ImGui::Button("Back", btnSize))
        m_screen = Screen::HOME;

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();
    /* ---- END ---- */
}

// scrollable list of existing games in /projects/Games/
void EditorLaunch::renderOpenScreen(const LaunchLayout& L) {
    const float  listW = 400.f;
    const float  listH = std::min(L.winSize.y * 0.45f, 320.f);
    const float  listY = L.contentY + 20.f;

    // label
    ImGui::SetCursorPos(ImVec2(L.centerX - listW * 0.5f, listY));
    ImGui::TextColored(ImVec4(0.85f, 0.85f, 0.85f, 1.f), "Select a Game:");

    /* ---- Scrollable list ---- */
    ImGui::SetCursorPos(ImVec2(L.centerX - listW * 0.5f, listY + 28.f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.14f, 0.14f, 0.18f, 1.f));
    ImGui::BeginChild("##gamelist", ImVec2(listW, listH), true);

    if (m_existingGames.empty()) {
        ImGui::TextColored(ImVec4(0.55f, 0.55f, 0.55f, 1.f), "No games found in projects/Games/");
    }

    for (const auto& gname : m_existingGames) {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.18f, 0.24f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.26f, 0.46f, 0.76f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.38f, 0.65f, 1.f));

        if (ImGui::Button(gname.c_str(), ImVec2(listW - 16.0f, 40.f)))
            confirmSelection(gname);

        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    /* ---- END ---- */

    if (!m_errorMsg.empty()) {
        ImGui::SetCursorPos(ImVec2(L.centerX - listW * 0.5f, listY + 28.f + listH + 8.f));
        ImGui::TextColored(ImVec4(1.f, 0.35f, 0.35f, 1.f), "%s", m_errorMsg.c_str());
    }

    /* ---- Back Button ---- */
    const float  backY   = listY + 28.f + listH + 18.f;
    const ImVec2 btnSize(150.f, 44.f);
    ImGui::SetCursorPos(ImVec2(L.centerX - btnSize.x * 0.5f, backY));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.35f, 0.4f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.45f, 0.45f, 0.52f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.28f, 0.28f, 0.33f, 1.f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.f);

    if (ImGui::Button("Back", btnSize))
        m_screen = Screen::HOME;

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();
    /* ---- END ---- */
}

// validates name 
void EditorLaunch::handleCreateConfirm() {
    std::string name = m_newNameBuf;

    // strip leading/trailing whitespace
    name.erase(0, name.find_first_not_of(" \t"));
    name.erase(name.find_last_not_of(" \t") + 1);

    if (name.empty()) {
        m_errorMsg = "Name cannot be empty.";
        return;
    }

    if (name.find_first_of("/\\:*?\"<>|") != std::string::npos) {
        m_errorMsg = "Name contains invalid characters.";
        return;
    }

    if (createNewGame(name)) confirmSelection(name);
}

// helper func to scan games in projects/Games/
void EditorLaunch::scanExistingGames() {
    m_existingGames.clear();
    m_errorMsg.clear();

    std::error_code ec;
    fs::path absGamesDir = fs::absolute(GAMES_DIR, ec);
    std::cout << "[EditorLaunch] Scanning: " << absGamesDir.string() << "\n";
    if (!fs::exists(GAMES_DIR, ec) || !fs::is_directory(GAMES_DIR, ec)) {
        m_errorMsg = std::string("Cannot find: ") + GAMES_DIR;
        return;
    }

    for (const std::filesystem::directory_entry& entry : fs::directory_iterator(GAMES_DIR, ec)) {
        if (entry.is_directory())
            m_existingGames.push_back(entry.path().filename().string());
    }

    std::sort(m_existingGames.begin(), m_existingGames.end());
}

// helper func to create a new game (folder with some basic stuff)
bool EditorLaunch::createNewGame(const std::string& name) {
    m_errorMsg.clear();

    fs::path root = fs::path(GAMES_DIR) / name;

    if (fs::exists(root)) {
        m_errorMsg = "A game named '" + name + "' already exists.";
        return false;
    }

    // folders to create
    const std::vector<fs::path> dirs = {
        "src",
        "include",
        "assets/Audio",
        "assets/fonts",
        "assets/Logic",
        "assets/Prefab",
        "assets/resources",
        "assets/Scene",
        "assets/scripting",
        //"assets/shaders",
        "assets/Texture",
    };

    std::error_code ec;
    for (const auto& d : dirs) {
        if (!fs::create_directories(root / d, ec)) {
            if (ec) {
                m_errorMsg = "Failed to create folder: " + (root / d).string();
                return false;   
            }
        }
    }

    fs::path engineAssets = fs::path(TANUKI_ENGINE_ASSETS_R);
    fs::path gameAssets = root / "assets";

    if (fs::exists(engineAssets)) {
        fs::copy(engineAssets, gameAssets,
            fs::copy_options::recursive | fs::copy_options::overwrite_existing, ec);
        if (ec) {
            m_errorMsg = "Failed to copy engine shaders: " + ec.message();
            return false;
        }
        std::cout << "[EditorLaunch] Copied engine shaders to: " << gameAssets.string() << "\n";
    }
    else {
        std::cerr << "[EditorLauncher] WARNING: Engine assets not found at: "
            << engineAssets.string() << "\n";
        std::cerr << "[EditorLauncher] New game may be missing shaders/fonts!\n";
    }

    // create main.cpp in /src
    std::ofstream f(root/"src"/"main.cpp");
    if (!f) { m_errorMsg = "Failed to create main.cpp"; return false; }
    f << "// " << name << " game entry point\n";
    f << "// TODO: register game scripts with CoreEngine here\n";

    // create new scene new.json
    static constexpr const char* EMPTY_SCENE_JSON = R"({
        "layers": [
            {"id": 0, "name": "Background", "enabled": 1, "visible": 1, "updatable": 1},
            {"id": 1, "name": "Game",       "enabled": 1, "visible": 1, "updatable": 1},
            {"id": 2, "name": "UI",         "enabled": 1, "visible": 1, "updatable": 1}
        ],
        "objects": []
    })";
    std::ofstream f2(root/"assets"/"Scene"/"new.json");
    if (!f2) { m_errorMsg = "Failed to create new.json"; return false; }
    f2 << EMPTY_SCENE_JSON << "\n";

    // create new config.json 
    std::ofstream cfg(root /"assets"/"resources"/"config.json");
    if (!cfg) { m_errorMsg = "Failed to create config.json"; return false; }
    cfg << "{\n";
    cfg << "  \"window\": {\n";
    cfg << "    \"width\": 1600,\n";
    cfg << "    \"height\": 800,\n";
    cfg << "    \"title\": \"" << name << "\",\n";
    cfg << "    \"vsync\": true,\n";
    cfg << "    \"fullscreen\": true\n";
    cfg << "  },\n";
    cfg << "  \"render\": {\n";
    cfg << "    \"clear_color\": [0.2, 0.2, 0.25, 1.0]\n";
    cfg << "  },\n";
    cfg << "  \"debug\": {\n";
    cfg << "    \"show_fps_in_title\": true,\n";
    cfg << "    \"show_input_debug\": true\n";
    cfg << "  }\n";
    cfg << "}\n";

    // create empty prefab registry
    static constexpr const char* EMPTY_PREFAB_REGISTRY = R"({
        "version": 1,
        "prefabIndex": []
    })";
    std::ofstream f3(root/"assets"/"Prefab"/"prefab_registry.json");
    if (!f3) { m_errorMsg = "Failed to create prefab_registry.json"; return false; }
    f3 << EMPTY_PREFAB_REGISTRY << "\n";

    std::cout << "[EditorLauncher] Created new game: " << root.string() << "\n";
    return true;
}

// confirm select
void EditorLaunch::confirmSelection(const std::string& name) {
    m_gameName = name;

    std::error_code ec;
    fs::path absPath = fs::absolute(fs::path(GAMES_DIR) / name, ec);
    m_gamePath = absPath.string();

    fs::path assetsPath = absPath/"assets";

    if (!fs::exists(assetsPath)) {
        m_errorMsg = "No assets folder found in: " + m_gamePath;
        return;
    }

    //int result = _chdir(m_gamePath.c_str());

    /*if (result != 0) {
        m_errorMsg = "Failed to set working directory to: " + m_gamePath;
        std::cerr << "[EditorLauncher] " << m_errorMsg << "\n";
        return;
    }*/

    // find first scene to open by default
    fs::path scenePath = absPath/"assets"/"Scene";
    if (fs::exists(scenePath)) {
        for (const auto& entry : fs::directory_iterator(scenePath, ec)) {
            if (entry.path().extension() == ".json") {
                m_defaultScene = entry.path().filename().string();
                break;
            }
        }
    }

    //m_gameDLLPath = "WaterBoundDLL.dll";

    std::cout << "[EditorLaunch] Opened game: " << name << "  (path: " << m_gamePath << ")\n";

    m_done = true;
}

//#endif