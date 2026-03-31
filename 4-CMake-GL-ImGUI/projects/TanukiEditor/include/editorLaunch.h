/* Start Header ************************************************************************/
/*!
\file       EditorLaunch.h
\author     Tan Chee Qing, cheeqing.tan, 2401486
            - 100%
\par		cheeqing.tan@digipen.edu
\date       March, 10th, 2026
\brief      A singleton full-screen ImGui launcher shown before the editor loads a project.
            Allows the user to create a new game project or open an existing one.
            Call EditorLauncher::render() each frame; it returns true once the
            user has made a selection and the launcher should be dismissed.

Copyright (C) 2026 DigiPen Institute of Technology.
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

/*!
 * \class EditorLaunch
 * \brief Singleton class that manages the editor launch process.
 *
 * Handles UI for creating new projects, opening existing ones,
 * and selecting the default scene for the engine to run.
 * Encapsulates launcher state and input handling.
 */
class EditorLaunch {
public:
    /*!
    * \brief Returns the singleton instance of EditorLaunch.
    *
    * Ensures only one launcher exists during the application's lifetime.
    *
    * \return Reference to the EditorLaunch singleton instance.
    */
    static EditorLaunch& Instance() {
        static EditorLaunch inst;
        return inst;
    }

    /*!
     * \brief Deleted copy constructor to enforce singleton pattern.
     */
    EditorLaunch(const EditorLaunch&)            = delete;

    /*!
     * \brief Deleted copy assignment operator to enforce singleton pattern.
     */
    EditorLaunch& operator=(const EditorLaunch&) = delete;

    /*!
     * \brief Renders the editor launcher UI for the current frame.
     *
     * Handles navigation between HOME, CREATE_INPUT, and OPEN_LIST screens,
     * processes user input and updates internal state.
     *
     * \return True if a project has been selected and the engine can proceed
     *         to Init/Run, otherwise false.
     */
    bool render();

    /*!
     * \brief Returns the absolute path of the selected game project.
     *
     * Valid only after a project has been chosen via the launcher.
     *
     * \return Reference to the string containing the game path.
     */
    const std::string& getGamePath() const { return m_gamePath; }

    /*!
     * \brief Returns the name of the selected game project (folder name).
     *
     * Used for display and internal identification purposes.
     *
     * \return Reference to the string containing the game name.
     */
    const std::string& getGameName() const { return m_gameName; }

    /*!
     * \brief Returns the default scene path of the selected game.
     *
     * Typically points to the first scene inside the /scene directory.
     *
     * \return Reference to the string containing the default scene path.
     */
    const std::string& getDefaultScene() const { return m_defaultScene; }

private:
    /*!
     * \brief Private constructor to initialize launcher state.
     *
     * Initializes internal variables and sets the starting screen to HOME.
     * Private to enforce singleton usage.
     */
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
    /*!
     * \brief Renders the HOME screen UI.
     *
     * Displays buttons for "Create New Game" and "Open Existing Game".
     * Handles navigation to other screens based on user input.
     *
     * \param L Layout configuration struct for rendering.
     */
    void renderHomeScreen(const struct LaunchLayout& L);

    /*!
     * \brief Renders the CREATE_INPUT screen UI.
     *
     * Allows the user to input a new game name.
     * Displays validation errors and confirmation options.
     *
     * \param L Layout configuration struct for rendering.
     */
    void renderCreateScreen(const struct LaunchLayout& L);

    /*!
     * \brief Renders the OPEN_LIST screen UI.
     *
     * Shows a scrollable list of existing game projects.
     * Allows the user to select a project for editing.
     *
     * \param L Layout configuration struct for rendering.
     */
    void renderOpenScreen(const struct LaunchLayout& L);

    /*!
     * \brief Handles confirmation logic when creating a new game.
     *
     * Validates the user input, attempts to create a new project folder,
     * and updates error messages if creation fails.
     */
    void handleCreateConfirm();

    /* ---- helpers ---- */
    /*!
     * \brief Scans the games directory for existing projects.
     *
     * Populates m_existingGames with the names of all available projects.
     * Called when entering the OPEN_LIST screen.
     */
    void scanExistingGames();

    /*!
    * \brief Creates a new game project folder structure.
    *
    * Sets up necessary subdirectories and starter files for a new project.
    *
    * \param name Name of the new game project.
    * \return True if creation succeeded, false if there was an error
    *         (e.g., duplicate name, filesystem error).
    */
    bool createNewGame(const std::string& name); // creates folder scaffold, returns false on error

    /*!
     * \brief Finalizes the selection of a game project.
     *
     * Sets the game path, name, and default scene.
     * Marks the launcher as done so the engine can proceed.
     *
     * \param name Name of the selected game project.
     */
    void confirmSelection(const std::string& name);

    static constexpr const char* GAMES_DIR = TANUKI_GAMES_ROOT_R;
};
//#endif