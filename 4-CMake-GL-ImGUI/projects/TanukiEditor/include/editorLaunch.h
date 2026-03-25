/* Star/* Start Header ************************************************************************/
/*!
\file       EditorLaunch.cpp
\author     Tan Chee Qing, cheeqing.tan, 2401486
            - 100%
\par		cheeqing.tan@digipen.edu
\date       March, 10th, 2026
\brief      Full-screen ImGui launcher shown before the editor loads a project.
            Allows the user to create a new game project or open an existing one.
            Call EditorLauncher::render() each frame; it returns true once the
            user has made a selection and the launcher should be dismissed.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

//#ifdef TANUKI_EDITOR   
#pragma once

#include <string>
#include <vector>
#ifdef _DEBUG
#include "Editor/editorState.h"
#endif

class EditorLaunch {
public:
    // -----------------------------------------------------------------------
    // Singleton
    // -----------------------------------------------------------------------
    static EditorLaunch& Instance() {
        static EditorLaunch inst;
        return inst;
    }
    EditorLaunch(const EditorLaunch&)            = delete;
    EditorLaunch& operator=(const EditorLaunch&) = delete;

    /* -----------------------------------------------------------------------
    // Call once per frame inside the launcher loop.
    // Returns true when the user has chosen a project and the engine should
    // proceed into its normal Init / Run path.
    // ----------------------------------------------------------------------- */
    bool render();

    /* -- absolute path to the chosen game's root folder -- */
    const std::string& getGamePath() const { return m_gamePath; }

    /* -- game name (folder name) -- */
    const std::string& getGameName() const { return m_gameName; }

    /* ---- open first scene in /scene by default -- */
    const std::string& getDefaultScene() const { return m_defaultScene; }

    //const std::string& getGameDLLPath() const { return m_gameDLLPath; }

private:
    EditorLaunch();

    /* ---- state ---- */
    enum class Screen {
        HOME,           // two big buttons
        CREATE_INPUT,   // text field: enter new game name
        OPEN_LIST       // scrollable list of existing games
    };
    Screen m_screen{ Screen::HOME };

    /* ---- data ---- */
    std::string m_gamePath;
    std::string m_gameName;
    std::string m_defaultScene;
    //std::string m_gameDLLPath;
    std::vector<std::string> m_existingGames; // populated when entering OPEN_LIST
    char m_newNameBuf[128]{};
    std::string m_errorMsg;
    bool m_done{ false };

    // helper renderer
    void renderHomeScreen(const struct LaunchLayout& L);
    void renderCreateScreen(const struct LaunchLayout& L);
    void renderOpenScreen(const struct LaunchLayout& L);

    // input handling
    void handleCreateConfirm();

    /* ---- helpers ---- */
    void scanExistingGames();
    bool createNewGame(const std::string& name); // creates folder scaffold, returns false on error
    void confirmSelection(const std::string& name);

    /* ---- path helpers ----  */
    // Root of projects/Games/ relative to the working directory the exe runs from.
    // Adjust SOURCE_GAMES_DIR if dff layout 
//#ifndef TANUKI_GAMES_ROOT_R
//    #define TANUKI_GAMES_ROOT_R "../Games"
//#endif
    static constexpr const char* GAMES_DIR = TANUKI_GAMES_ROOT_R;
};
//#endif