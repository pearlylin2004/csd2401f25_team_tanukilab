/* Start Header ************************************************************************/
/*!
\file       ui.cpp
\author     Tan Chee Qing, cheeqing.tan, 2401486
            - 90%
            Pearly Lin Lee Ying, p.lin, 2401591
            - 10% of the file
\par        p.lin@digipen.edu
\par		cheeqing.tan@digipen.edu
\date       September, 17th, 2025
\brief      definition of functions, drawPerformance, drawControls, drawDebug, shapeControl
            for the imgui windows

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "Editor/editorManager.h"

EditorManager::EditorManager() :
    assetBrowser(Editor::assetBrowserState, Editor::objSelectionState, Editor::menuBarState),
    editorDebugWindow(DebugMode::Editor),
    playDebugWindow(DebugMode::PlaySimul),
    hierarchyWindow(Editor::objSelectionState),
    inspectorWindow(Editor::objSelectionState),
    menuBar(assetBrowser, Editor::assetBrowserState, Editor::menuBarState, Editor::objSelectionState, Editor::sceneState),
    performanceWindow(),
    sceneWindow(assetBrowser, Editor::objSelectionState, Editor::menuBarState, Editor::sceneWindowState, Editor::sceneState) {}

void EditorManager::update(GameObjectManager& manager) {
    if (assetsChangedFlag) {
        assetBrowser.loadAssetsFromDirectory();
        assetsChangedFlag = false;
    }


    sceneWindow.objPicking(manager);
    sceneWindow.objDragging();

    assetBrowser.render();
    editorDebugWindow.render();
    playDebugWindow.render();
    hierarchyWindow.render(manager);
    inspectorWindow.render(manager);
    menuBar.render(manager);
    performanceWindow.render();

    // only show play info when not editing
    if (!isEditing) playDebugWindow.render();
}

void EditorManager::renderScene(GLuint texture, float aspectRatio, GameObjectManager& manager) {
    sceneWindow.render(texture, aspectRatio, manager);
}

void EditorManager::handleShortcutKeys(GameObjectManager& manager) {
    // CTRL + S -> save scene
    if (InputHandler::isComboKeyTriggered(GLFW_KEY_S, GLFW_KEY_LEFT_CONTROL) ||
        InputHandler::isComboKeyTriggered(GLFW_KEY_S, GLFW_KEY_RIGHT_CONTROL)) {
        if (isEditingMode()) {
            manager.saveScene(JsonIO::sourceScenePath(sceneWindow.getCurrentSceneName()));
            JsonIO::syncSceneToRuntime(sceneWindow.getCurrentSceneName());
        }
        else {
            DebugLog::addMessage("Playing simulation. File cannot be saved.");
        }
    }


    // F5 -> Play/Stop Simulation
    if (InputHandler::isKeyTriggered(GLFW_KEY_F5)) {
        menuBar.toggleSimulation(manager);
    }
}