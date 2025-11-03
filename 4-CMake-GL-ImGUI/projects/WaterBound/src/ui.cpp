/* Start Header ************************************************************************/
/*!
\file       ui.cpp
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 100% of the file
\par        p.lin@digipen.edu
\date       September, 17th, 2025
\brief      definition of functions, drawPerformance, drawControls, drawDebug, shapeControl
            for the imgui windows

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "ui.h"
#include <cstdlib>

#ifndef RUNTIME_SCENE_DIR_R
#define RUNTIME_SCENE_DIR_R "./shaders/Scene"
#endif
#ifndef SOURCE_SCENE_DIR_R
#define SOURCE_SCENE_DIR_R "./projects/WaterBound/shaders/Scene"
#endif
#ifndef SOURCE_DIR_R
#define SOURCE_DIR_R "./projects/WaterBound/shaders"
#endif
static std::string runtimeScenePath(std::string const& name) {
    return std::string(RUNTIME_SCENE_DIR_R) + "/" + name;
}
static std::string sourceScenePath(std::string const& name) {
    return std::string(SOURCE_SCENE_DIR_R) + "/" + name;
}
static bool syncSceneToRuntime(const std::string& name) {
    const std::string src = sourceScenePath(name);
    const std::string dst = runtimeScenePath(name);

    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path(dst).parent_path(), ec);
    std::filesystem::copy_file(
        src, dst,
        std::filesystem::copy_options::overwrite_existing,
        ec
    );

    if (ec) {
        std::cerr << "Scene sync failed: " << src << " -> " << dst
            << " (" << ec.message() << ")\n";
        return false;
    }
    std::cout << "Scene synced: " << dst << "\n";
    return true;
}
bool UI::assetsChangedFlag = true;

UI::UI() {

    //m_currentSceneName = "shaders/Scene/level01.json";
    ////m_currentSceneName = "shaders/Scene/temp_serialize.txt";
    ////m_tempSceneName = "shaders/Scene/temp_scene";
    //m_tempSceneName = "shaders/Scene/temp_scene.json";
    m_currentSceneName = "level01.json";
    m_tempSceneName = "temp_scene.json";
    m_isPlaying = false;
}

void UI::update(GameObjectManager& manager) {
    if (InputHandler::isComboKeyTriggered(GLFW_KEY_S, GLFW_KEY_LEFT_CONTROL) ||
        InputHandler::isComboKeyTriggered(GLFW_KEY_S, GLFW_KEY_RIGHT_CONTROL)) {
        manager.saveScene(sourceScenePath(m_currentSceneName));
        syncSceneToRuntime(m_currentSceneName);
    }

    editorMenuBar(manager);
    objPicking(manager);
    objDragging();

    assetWindow(manager);
    drawPerformance();
    drawDebug();

    objHierarchyWindow(manager);
    objInspectorWindow(manager);
    editorCameraControls();
    drawShapes(manager);

    // conditional popup
	newScenePopup(manager); // popup for creating new scene
	duplicateScenePopup(manager); // popup for duplicating scene
	loadScenePopup(manager); // popup for loading scene
	assetDeletePopup(); // popup for confirming asset deletion
	assetRenamePopup(); // popup for renaming asset
	assetReplacePopup(); // popup for replacing asset
}

void UI::addDebugMessage(const std::string& msg) {
    m_debugMessages.push_back(msg);
}

void UI::drawPerformance() {
    ImGui::Begin("Performance");
    //time frame:
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
        1000.0f / ImGui::GetIO().Framerate,
        ImGui::GetIO().Framerate);
    ImGui::Separator();
    //average time --can add for other functions also just need to add 4 lines of codes into the function start and end(see InputSystem::Update in system.cpp)
    double totalMs = 0.0;
    for (auto& timer : g_SystemTimers) totalMs += timer.ms;
    //loop through all timers and display
    for (auto& timer : g_SystemTimers) {
        float percent = totalMs > 0.0 ? (float)((timer.ms / totalMs) * 100.0) : 0.0f;
        ImGui::Text("%s: %.3f ms (%.1f%%)", timer.name.c_str(), timer.ms, percent);
        ImGui::ProgressBar(percent / 100.0f, ImVec2(0.0f, 0.0f));
    }
    g_SystemTimers.clear(); // clear timers for next frame
    ImGui::End();
}

void UI::drawShapes(GameObjectManager& manager) {
    {
        ImGui::Begin("Create Object");

		//// spawning objects (double up every click)
        static int count = 0;
		static int twentyCount = 0;
        if (ImGui::Button("Spawn Object")) {
			int spawnCount = (count == 0) ? 1 : count * 2;
			if (spawnCount == 2048)
				spawnCount = 2500; //cap at 400 objects

			for (int i = count; i < spawnCount; ++i) {
				std::string cloneName = "Player_Clone(" + std::to_string(i + 1) + ")";
			    GameObject* newObj = manager.CloneGameObject("player2", cloneName);
				
                Transform* transform = newObj->getComponent<Transform>();

                int randomX = rand() % 20;
                int randomY = rand() % 10;
				int rotation = rand() % 360;
				int scaleX = (rand() % 3) + 1;
				int scaleY = (rand() % 3) + 1;

				if (rand() % 2 == 0)
					randomX = -randomX;
				if (rand() % 2 == 1)
					randomY = -randomY;

                transform->x = randomX;
                transform->y = randomY;
				transform->rotation = rotation;
				transform->scaleX = scaleX;
				transform->scaleY = scaleY;
			}

			count = spawnCount;
        }


        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        //ImGui::PushItemWidth(150.0f);

        // For drawing circle
        ImGui::Text("X");
        ImGui::SameLine();
        ImGui::InputFloat("##X", &circleInput.inputX, 0.1f);

        ImGui::Text("Y");
        ImGui::SameLine();
        ImGui::InputFloat("##Y", &circleInput.inputY, 0.1f);

        ImGui::Text("Z");
        ImGui::SameLine();
        ImGui::InputFloat("##Z", &circleInput.inputZ, 0.1f);
        ImGui::Spacing();

        //ImGui::PopItemWidth();

        if (ImGui::Button("Add Circle")) {
            std::vector<GameObject*> gameObjects;
            manager.getAllGameObjects(gameObjects);

            GameObject* circleObj = manager.createGameObject("Circle_" + std::to_string(gameObjects.size()));

            Transform* transform = circleObj->addComponent<Transform>();
            Render* render = circleObj->addComponent<Render>();
            render->modelRef = renderer::models[static_cast<int>(shape::circle) + static_cast<int>(shape::circle)];
            //render->modelRef = renderer::models[static_cast<int>(shape::circle)];
            transform->x = circleInput.inputX;
            transform->y = circleInput.inputY;
            transform->z = circleInput.inputZ;

            //auto* coll = circleObj->addComponent<CollisionInfo>();
            addDebugMessage("Circle created at (" + std::to_string(transform->x) + ", " + std::to_string(transform->y) + ", " + std::to_string(transform->z) + ")\n");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // For drawing rectangle        
        ImGui::Text("X");
        ImGui::SameLine();
        ImGui::InputFloat("##X1", &RectangleInput.inputX, 0.1f);

        ImGui::Text("Y");
        ImGui::SameLine();
        ImGui::InputFloat("##Y1", &RectangleInput.inputY, 0.1f);

        ImGui::Text("Z");
        ImGui::SameLine();
        ImGui::InputFloat("##Z1", &RectangleInput.inputZ, 0.1f);
        ImGui::Spacing();

        if (ImGui::Button("Add Rectangle")) {
            std::vector<GameObject*> gameObjects;
            manager.getAllGameObjects(gameObjects);

            GameObject* recobj = manager.createGameObject("Rectangle_" + std::to_string(gameObjects.size()));

            Transform* transform = recobj->addComponent<Transform>();
            Render* render = recobj->addComponent<Render>();
            render->modelRef = renderer::models[static_cast<int>(shape::square)];
            //render->modelRef = renderer::setup_square({ 1.0f, 0.f, 0.f });

            transform->x = RectangleInput.inputX;
            transform->y = RectangleInput.inputY;
            transform->z = RectangleInput.inputZ;

            //auto* col = recobj->addComponent<CollisionInfo>();
            addDebugMessage("Rectangle created at (" + std::to_string(transform->x) + ", " + std::to_string(transform->y) + ", " + std::to_string(transform->z) + ")\n");
        }

        ImGui::End();
    }
}

void UI::drawDebug() {
    ImGui::Begin("Debug Info");

    for (const auto& msg : m_debugMessages) {
        ImGui::Text("%s", msg.c_str());
    }

    //auto-scroll if at the bottom
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::End();
}

void UI::editorMenuBar(GameObjectManager& manager) {
    ImGui::BeginMainMenuBar();

    /* ------------------- File Menu ------------------- */
    if (ImGui::BeginMenu("File")) {

        // create new scene
        if (ImGui::MenuItem("New Scene")) {
            m_MenuBarState.showNewScenePopup = true;
        }

        // duplicate scene
        if (ImGui::MenuItem("Duplicate Scene")) {
            m_MenuBarState.showDuplicatePopup = true;
        }

        // save current scene to file
        if (ImGui::MenuItem("Save Scene")) {
            //manager.saveScene(m_currentSceneName);
            manager.saveScene(sourceScenePath(m_currentSceneName));
            syncSceneToRuntime(m_currentSceneName);
        }

        // load an existing scene
        if (ImGui::BeginMenu("Load Scene")) {

            // load through the assets to find scene files (all files saved in "Scene" folder)
            for (const Asset& asset : m_assets) {
                if (asset.folder == m_AssetBrowserState.currentFolder + "Scene") {
					// create a submenu for the scenes
                    if (ImGui::MenuItem(asset.name.c_str())) {
                        m_MenuBarState.sceneToLoad = asset.name;
						m_MenuBarState.showLoadScenePopup = true;
                    }
                }
            }
            ImGui::EndMenu();

        }

        ImGui::EndMenu();
    }

    /* ------------------- Simulation ------------------- */
    if (ImGui::BeginMenu("Simulation")) {
        if (!m_isPlaying) {
            if (ImGui::MenuItem("Play")) {
                m_isPlaying = true;

                // save current state to a temp scene file
                //manager.saveScene(m_tempSceneName);
                manager.saveScene(runtimeScenePath(m_tempSceneName));

                addDebugMessage("Simulation started.\n");
            }
        }
        else {
            if (ImGui::MenuItem("Stop")) {
                m_isPlaying = false;

                // load back the temp scene file to restore state
                manager.loadScene(runtimeScenePath(m_tempSceneName));

                m_SelectionState.selectedObject = nullptr;
                m_SelectionState.draggedObject = nullptr;
                m_SelectionState.selectedIndex = -1;

                // remove temp scene file
                //if (std::filesystem::exists(m_tempSceneName))
                //    std::filesystem::remove(m_tempSceneName);
                const auto tempFull = runtimeScenePath(m_tempSceneName);
                if (std::filesystem::exists(tempFull))
                    std::filesystem::remove(tempFull);
                addDebugMessage("Simulation stopped.\n");
            }
        }
        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}

void UI::newScenePopup(GameObjectManager& manager) {
    // open popup if flagged
    if (m_MenuBarState.showNewScenePopup) {
        ImGui::OpenPopup("Confirm New Scene");
        m_MenuBarState.showNewScenePopup = false;
    }

    if (ImGui::BeginPopupModal("Confirm New Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Create a new scene? Unsaved changes will be lost.");

        // Save and open new scene
        if (ImGui::Button("Save and Create New")) {
            //manager.saveToFile(m_currentSceneName);
            manager.saveScene(sourceScenePath(m_currentSceneName));
            syncSceneToRuntime(m_currentSceneName);

            // create new scene
            newScene(manager);

            // load new scene
            m_MenuBarState.sceneToLoad = m_currentSceneName;
			loadScene(manager);

            ImGui::CloseCurrentPopup();
        }

        // Discard changes and open new scene
        ImGui::SameLine();
        if (ImGui::Button("Discard Changes")) {

            // create new scene
            newScene(manager);

            // load new scene
            m_MenuBarState.sceneToLoad = m_currentSceneName;
            loadScene(manager);
            ImGui::CloseCurrentPopup();
        }

        // Cancel duplicate and stay in current scene
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void UI::duplicateScenePopup(GameObjectManager& manager) {
    if (m_MenuBarState.showDuplicatePopup) {
        m_MenuBarState.showDuplicatePopup = false;
        ImGui::OpenPopup("Confirm Duplicate Scene");
    }

	if (ImGui::BeginPopupModal("Confirm Duplicate Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Duplicate and open new a scene? Unsaved changes will be lost.");

        // Save and open newly duplicated scene
        if (ImGui::Button("Save and Duplicate")) {
            //manager.saveToFile(m_currentSceneName);
            manager.saveScene(sourceScenePath(m_currentSceneName));
            syncSceneToRuntime(m_currentSceneName);
           
            duplicateScene(manager);
            ImGui::CloseCurrentPopup();
        }

		// Discard changes and open newly duplicated scene
        ImGui::SameLine();
        if (ImGui::Button("Discard Changes")) {

            duplicateScene(manager);
            ImGui::CloseCurrentPopup();
        }

		// Cancel duplicate and stay in current scene
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

		ImGui::EndPopup();
	}
}

void UI::loadScenePopup(GameObjectManager& manager) {
    if (m_MenuBarState.showLoadScenePopup) {
        ImGui::OpenPopup("Confirm Load Scene");
        m_MenuBarState.showLoadScenePopup = false;
    }

    if (ImGui::BeginPopupModal("Confirm Load Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		// show different title if reloading current scene
        std::string title = (m_MenuBarState.sceneToLoad == m_currentSceneName) ? "Reload current scene? Unsaved changes will be lost." : "Load another scene? Unsaved changes will be lost";
        
        ImGui::Text(title.c_str());

        // Save and load another scene
        if (ImGui::Button("Save and Load")) {
            //manager.saveToFile(m_currentSceneName);
            manager.saveScene(sourceScenePath(m_currentSceneName));
            syncSceneToRuntime(m_currentSceneName);

            loadScene(manager);
            ImGui::CloseCurrentPopup();
        }

        // Discard changes and load selected scene
        ImGui::SameLine();
        if (ImGui::Button("Discard Changes")) {
            loadScene(manager);
            ImGui::CloseCurrentPopup();
        }

        // Cancel loading and stay in current scene
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void UI::newScene(GameObjectManager& manager) {
    //std::string folder = std::string(SOURCE_SCENE_DIR_R);
	std::string base = "untitled_scene";
	std::string extension = ".json";
    std::string newSceneName;

    int copy = 1;
	bool nameExists;
    do {
        nameExists = false;
        newSceneName = base + "(" + std::to_string(copy) + ")" + extension;

        for (const Asset& asset : m_assets) {
            if (asset.name == std::filesystem::path(newSceneName).filename().string()) {
                nameExists = true;
                break;
            }
        }

        copy++;
    } while (nameExists);

    m_currentSceneName = newSceneName;
    manager.saveScene(sourceScenePath(m_currentSceneName), true);
    syncSceneToRuntime(m_currentSceneName);
    assetsChangedFlag = true;

    m_SelectionState.selectedObject = nullptr;
    m_SelectionState.draggedObject = nullptr;
    m_SelectionState.selectedIndex = -1;

    addDebugMessage("New scene " + m_currentSceneName + " created.\n");
}

void UI::duplicateScene(GameObjectManager& manager) {
    if (!m_currentSceneName.empty()) {
		std::filesystem::path originalName(m_currentSceneName);
		std::string folder = originalName.parent_path().string();
		std::string filename = originalName.stem().string();
		std::string extension = originalName.extension().string();

		// check for existing names and find a new unique name
        std::string newSceneName;
        int copy = 1;
        bool nameExists;
        do {
			nameExists = false;
			newSceneName = folder + "/" + filename + "_copy(" + std::to_string(copy) + ")" + extension;

            for (const Asset& asset : m_assets) {
                if (asset.name == std::filesystem::path(newSceneName).filename().string()) {
					nameExists = true;
					break;
                }
            }

			copy++;
        } while (nameExists);
	
        //manager.saveToFile(newSceneName);
        m_currentSceneName = newSceneName;
        manager.saveScene(sourceScenePath(m_currentSceneName));
        syncSceneToRuntime(m_currentSceneName);
        assetsChangedFlag = true;

        m_SelectionState.selectedObject = nullptr;
        m_SelectionState.draggedObject = nullptr;
        m_SelectionState.selectedIndex = -1;
        addDebugMessage("Scene duplicated to: " + newSceneName + "\n");
    }
    else {
        addDebugMessage("No current scene to duplicate.\n");
    }
}

void UI::loadScene(GameObjectManager & manager) {
    //manager.loadFromFile(m_MenuBarState.sceneToLoad);
    manager.loadScene(runtimeScenePath(m_MenuBarState.sceneToLoad));

    m_currentSceneName = std::move(m_MenuBarState.sceneToLoad);

    m_SelectionState.selectedObject = nullptr;
    m_SelectionState.draggedObject = nullptr;
    m_SelectionState.selectedIndex = -1;

    addDebugMessage("Loaded scene: " + m_currentSceneName + "\n");
}

void UI::objHierarchyWindow(GameObjectManager& manager) {

    ImGui::Begin("Hierarchy");

    std::vector<GameObject*> gameObjects;
    manager.getAllGameObjects(gameObjects);

    // by default just set to no children
    ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_Leaf;

    for (size_t i = 0; i < gameObjects.size(); i++) {
        // if no children -> in the future
    //    baseFlags |= ImGuiTreeNodeFlags_Leaf;

		// highlight selected row
        if (i == m_SelectionState.selectedIndex) {
            baseFlags |= ImGuiTreeNodeFlags_Selected;
        }
        else {
			baseFlags &= ~ImGuiTreeNodeFlags_Selected;
        }

		// draw tree node for each object
        if (ImGui::TreeNodeEx(gameObjects[i]->getObjectName().c_str(), baseFlags)) {
            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
                m_SelectionState.selectedIndex = static_cast<int>(i);
                m_SelectionState.selectedObject = gameObjects[i];
            }
            ImGui::TreePop();

        }
    }

    if (gameObjects.empty()) {
        m_SelectionState.selectedObject = nullptr;
        m_SelectionState.draggedObject = nullptr;
        m_SelectionState.selectedIndex = -1;
        ImGui::Text("No GameObjects available");
    }

	ImGui::End();
}

void UI::objInspectorWindow(GameObjectManager& manager) {
    ImGui::Begin("Inspector");

    if (m_SelectionState.selectedObject) {

        /* ------------------- Edit Object Name ------------------- */
        std::string& objName = m_SelectionState.selectedObject->getObjectName();
        char buffer[128];
        strncpy(buffer, objName.c_str(), sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';

        ImGui::Text("Name");
        ImGui::SameLine();
        if (ImGui::InputText("##Name", buffer, sizeof(buffer))) {
            std::string newObjName = buffer;
            if (manager.renameGameObject(m_SelectionState.selectedObject, newObjName)) {
                addDebugMessage("Object renamed to " + newObjName + "\n");
            }
            else {
                addDebugMessage("Failed to rename: name already exists\n");
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();


        /* ------------------- Object Transformation ------------------- */
        Transform* transform = m_SelectionState.selectedObject->getComponent<Transform>();
        if (ImGui::CollapsingHeader("Transform")) {

            ImGui::DragFloat("X", &transform->x);
            ImGui::DragFloat("Y", &transform->y);
            ImGui::DragFloat("Z", &transform->z);

            /*addDebugMessage("Transform updated: Position(" +
                std::to_string(transform->x) + ", " +
                std::to_string(transform->y) + ", " +
                std::to_string(transform->z) + ")\n");*/
        }

        /* ------------------- Object Rotation ------------------- */
        if (ImGui::CollapsingHeader("Rotation")) {
            ImGui::DragFloat("Angle", &transform->rotation);

            /*addDebugMessage("Transform updated: Rotation(" +
                std::to_string(transform->rotation) + ")\n");*/
        }

        /* ------------------- Object Scale ------------------- */
        if (ImGui::CollapsingHeader("Scale")) {
            ImGui::Checkbox("Aspect Ratio (X only)", &m_SelectionState.aspectRatioLock);

            if (m_SelectionState.aspectRatioLock && !m_SelectionState.ratioSet) {
                m_SelectionState.ratioY = transform->scaleY / transform->scaleX;
                m_SelectionState.ratioZ = transform->scaleZ / transform->scaleX;
                m_SelectionState.ratioSet = true;
            }

            if (!m_SelectionState.aspectRatioLock) m_SelectionState.ratioSet = false;

			// clamp scale to be non-negative
            ImGui::DragFloat("X1", &transform->scaleX, 0.1f, 0.0f, FLT_MAX);
            ImGui::DragFloat("Y1", &transform->scaleY, 0.1f, 0.0f, FLT_MAX);
            ImGui::DragFloat("Z1", &transform->scaleZ, 0.1f, 0.0f, FLT_MAX);

            if (m_SelectionState.aspectRatioLock && transform->scaleX != 0.0f) {
                transform->scaleY = transform->scaleX * m_SelectionState.ratioY;
                transform->scaleZ = transform->scaleX * m_SelectionState.ratioZ;
            }

            /*addDebugMessage("Transform updated: Position(" +
                std::to_string(transform->scaleX) + ", " +
                std::to_string(transform->scaleY) + ", " +
                std::to_string(transform->scaleZ) + ")\n");*/
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        /* ------------------- Change Texture ------------------- */
        // drag and drop received from asset window
        Render* render = m_SelectionState.selectedObject->getComponent<Render>();
        if (render && ImGui::CollapsingHeader("Render")) {
            ImGui::Text("Texture");
            ImGui::SameLine();

            std::string texName = render->texFile.empty() ? "(Drop new texture here)" : render->texFile;
            ImGui::Button(texName.c_str());

            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET")) {
                    const char* droppedPath = (const char*)payload->Data;
                    render->texFile = droppedPath;
                    render->hasTex = true;
                    render->texChanged = true;
                    addDebugMessage("Changed texture of " + m_SelectionState.selectedObject->getObjectName() + " to " + render->texFile + "\n");
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::SameLine();
            if (render->hasTex) {
                ImGui::Spacing();
                if (ImGui::Button("Clear Texture")) {
                    glDeleteTextures(1, &render->texHDL);
                    render->texHDL = 0;
                    render->hasTex = false;
                    render->texFile.clear();
                    render->texChanged = false;
                }
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        /* ------------------- Delete Object ------------------- */
        std::string name = m_SelectionState.selectedObject->getObjectName();
        if (ImGui::Button("Delete Object")) {
            ImGui::OpenPopup("Confirm Delete Object");
        }

        // delete confirm popup
        if (ImGui::BeginPopupModal("Confirm Delete Object", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Are you sure you want to delete this object?");

            if (ImGui::Button("Delete")) {

                manager.deleteGameObject(m_SelectionState.selectedObject);

				addDebugMessage("Object " + name + " deleted from manager\n");

                m_SelectionState.selectedObject = nullptr;
                m_SelectionState.draggedObject = nullptr;
                m_SelectionState.selectedIndex = -1;

                addDebugMessage("Object " + name + " deleted\n");

                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("No")) {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

    }
    else {
        ImGui::Text("No object selected");
    }

	ImGui::End();
}

void UI::loadAssetsFromDirectory() {
    if (assetsChangedFlag == false) return; // only reload if flagged
    m_assets.clear(); // clear existing assets before loading new ones

    // iterate over all files and folders in this directory and push all files to m_assets (flat list)
    for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(m_AssetBrowserState.currentFolder)) {
        if (entry.is_regular_file()) {
            Asset asset;
            asset.name = entry.path().filename().string();
            asset.path = entry.path().string();
            asset.folder = entry.path().parent_path().string(); // folder in assets that contains this file, for future filtering

            /*std::cout << "Asset Name: " << asset.name <<
				"\n Asset Path: " << asset.path <<
				"\n Asset Folder: " << asset.folder << std::endl;*/
            m_assets.push_back(asset);
        }
    }

    assetsChangedFlag = false;
    addDebugMessage("Assets loaded from " + m_AssetBrowserState.currentFolder + "\n");
}

void UI::assetWindow(GameObjectManager& manager) {
    ImGui::Begin("Assets");

    // refresh to update assets from the actual asset folder
    // not really needed since we have auto refresh, but still good to have
    if (ImGui::Button("Refresh")) {
        assetsChangedFlag = true;
        loadAssetsFromDirectory();
    }

    
    /* ------------------ Auto update if changes in project directory ---------------------*/
    // TEMPORARY
    // Check top-level folder got any updates (the asset folder)
    std::filesystem::file_time_type currentTopLevel = std::filesystem::last_write_time(m_AssetBrowserState.currentFolder);
    bool needsRefresh = false;

    if (currentTopLevel != m_AssetBrowserState.lastTopLevelRefresh) {
        m_AssetBrowserState.lastTopLevelRefresh = currentTopLevel;
        needsRefresh = true;
    }

    // Now check subfolders recursively (cheap guard: only if assetsChangedFlag is false)
    std::filesystem::file_time_type newestTime = m_AssetBrowserState.lastRefresh;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(m_AssetBrowserState.currentFolder)) {
        auto t = std::filesystem::last_write_time(entry);
        if (t > newestTime) {
            newestTime = t;
            needsRefresh = true;
        }
    }

    // update and reload if necessary
    if (needsRefresh) {
        m_AssetBrowserState.lastRefresh = newestTime;
        assetsChangedFlag = true;
        loadAssetsFromDirectory();
    }
    /* ------------------ End ---------------------*/


    ImGui::Separator();

    float windowVisibleX2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

    int index{};
    for (const Asset& asset : m_assets) {
        // skip temp scene file (for play/stop)
        std::string fullPath = asset.folder + "/" + asset.name;
        //if (fullPath == m_tempSceneName) continue;

        if (asset.name == m_tempSceneName) continue;

        ImGui::PushID(index++);

        // display button and the name of the file
        ImGui::BeginGroup();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(100, 40));

        // can consider changing to image in the future
        ImGui::Button(asset.name.c_str(), ImVec2(m_AssetBrowserState.iconSize, m_AssetBrowserState.iconSize));

        ImGui::PopStyleVar();

		// right click asset and show context menu
		if (ImGui::BeginPopupContextItem("AssetContextMenu")) {
            // show this asset in window explorer
			if (ImGui::MenuItem("Show in Explorer")) {
                // convert filepath to absolute to open in Window Explorer
                std::filesystem::path absPath = std::filesystem::absolute(fullPath);
                std::string absPathStr = absPath.string();

                std::replace(absPathStr.begin(), absPathStr.end(), '/', '\\');

                std::string command = "explorer /select,\"" + absPathStr + "\"";
                std::cout << "Running: " << command << std::endl;
				system(command.c_str());
			}

            // open popup window and ask whether to delete this current asset
			if (ImGui::MenuItem("Delete Asset")) {
				m_AssetBrowserState.fullPathToDelete = fullPath;
				m_AssetBrowserState.showDeletePopup = true;
			}

			// open popup window and rename this current asset
			if (ImGui::MenuItem("Rename Asset")) {
				m_AssetBrowserState.fullPathToRename = fullPath;
				m_AssetBrowserState.showRenamePopup = true;
				
				// pre-fill the current name
                std::strncpy(m_AssetBrowserState.newNameBuffer, asset.name.c_str(), sizeof(m_AssetBrowserState.newNameBuffer));
			}

			// open popup window and replace this current asset
			/*if (ImGui::MenuItem("Replace Asset")) {
                m_AssetBrowserState.fullPathToReplace = fullPath;
                m_AssetBrowserState.showReplacePopup = true;
                m_AssetBrowserState.replacePathBuffer[0] = '\0';
			}*/
			ImGui::EndPopup();
		}

		// double click logic to open scene files
        if (ImGui::IsItemHovered() && InputHandler::isMouseLeftDoubleClicked()) {
            // if it's a scene file, load the scene
            if (asset.folder == m_AssetBrowserState.currentFolder + "Scene") {
                m_MenuBarState.sceneToLoad = asset.name;
                m_MenuBarState.showLoadScenePopup = true;
            }
        }

        // make sure this text wraps if it's too long
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + m_AssetBrowserState.iconSize);
        ImGui::Text("%s", asset.name.c_str());
        ImGui::PopTextWrapPos();

        ImGui::EndGroup();

        // calculate whether the next button should be displayed in the same line or next line based on this window size
        float lastButtonX2 = ImGui::GetItemRectMax().x;
        float nextButtonX2 = lastButtonX2 + m_AssetBrowserState.iconSize;
        if (nextButtonX2 < windowVisibleX2) ImGui::SameLine();

        // drag a texture to inspector window to change texture of selected object
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            ImGui::SetDragDropPayload("ASSET", asset.path.c_str(), asset.path.size() + 1);
            ImGui::Text("%s", asset.name.c_str());
            ImGui::EndDragDropSource();
        }

        ImGui::PopID();
    }

	ImGui::End();
}

void UI::assetDeletePopup() {
	// open popup if flagged
    if (m_AssetBrowserState.showDeletePopup) {
        ImGui::OpenPopup("Confirm Delete Asset");
        m_AssetBrowserState.showDeletePopup = false;
    }

    /*---------------- Cannot delete current loaded scene ----------------*/
    if (!m_AssetBrowserState.fullPathToDelete.empty() &&
        m_currentSceneName == std::filesystem::path(m_AssetBrowserState.fullPathToDelete).filename().string())
    {
        ImGui::OpenPopup("Cannot Delete Current Scene");
        m_AssetBrowserState.fullPathToDelete.clear(); // clear so the check doesn't repeat
    }

    if (ImGui::BeginPopupModal("Cannot Delete Current Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Cannot delete the currently loaded scene!");

        if (ImGui::Button("OK")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
    /*-------------------- END -------------------*/


    /*---------------- Delete Confirmation ----------------*/
    if (ImGui::BeginPopupModal("Confirm Delete Asset", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to delete this asset?");

        if (ImGui::Button("Delete")) {

            if (!m_AssetBrowserState.fullPathToDelete.empty() &&
                std::filesystem::exists(m_AssetBrowserState.fullPathToDelete)) {
                std::filesystem::remove(m_AssetBrowserState.fullPathToDelete);
                assetsChangedFlag = true; // to reload assets

				// delete from the project folder as well
                std::filesystem::path fullPath(m_AssetBrowserState.fullPathToDelete);
                std::string filename = fullPath.filename().string();

                // split the "shaders/" part to get relative path
                std::string relativePath;
                auto pos = m_AssetBrowserState.fullPathToDelete.find("shaders/");
                if (pos != std::string::npos) {
                    relativePath = m_AssetBrowserState.fullPathToDelete.substr(pos + strlen("shaders/"));
                }

                // combine relative path to delete
                std::filesystem::path projectPath = std::string(SOURCE_DIR_R) + "/" + relativePath;
                if (std::filesystem::exists(projectPath)) {
                    std::filesystem::remove(projectPath);
                    std::cout << "Deleted asset (Project): " << projectPath.string() << std::endl;
                }

                addDebugMessage("Deleted asset: " + m_AssetBrowserState.fullPathToDelete + "\n");
            }

            m_AssetBrowserState.fullPathToDelete.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("No")) {
            m_AssetBrowserState.fullPathToDelete.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
    /*-------------------- END -------------------*/
}

void UI::assetRenamePopup() {
    // open popup if flagged
    if (m_AssetBrowserState.showRenamePopup) {
        ImGui::OpenPopup("Rename Asset");
        m_AssetBrowserState.showRenamePopup = false;
    }

    if (ImGui::BeginPopupModal("Rename Asset", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter new name for the asset");

        ImGui::InputText("##newName", m_AssetBrowserState.newNameBuffer, sizeof(m_AssetBrowserState.newNameBuffer));

        if (ImGui::Button("Rename")) {
            if (!m_AssetBrowserState.fullPathToRename.empty()) {
                std::filesystem::path old = m_AssetBrowserState.fullPathToRename;
                std::filesystem::path newPath = old.parent_path() / m_AssetBrowserState.newNameBuffer;

				// keep the extension if not provided
                if (newPath.extension().empty())
                    newPath.replace_extension(old.extension());

                if (!std::filesystem::exists(newPath)) {
                    std::filesystem::rename(old, newPath);
                    assetsChangedFlag = true; // reload assets

					// change in the project folder as well
                    std::filesystem::path sourceDir(SOURCE_DIR_R);
                    
					// split the "shaders/" part to get relative path
                    std::string relative;
                    auto pos = old.string().find("shaders/");
                    if (pos != std::string::npos) {
                        relative = old.string().substr(pos + strlen("shaders/"));
                    }
                    else {
                        relative = old.filename().string();
                    }

					// combine relative path to rename
                    std::filesystem::path projectOld = sourceDir / relative;
                    std::filesystem::path projectNew = projectOld.parent_path() / newPath.filename();
                    if (std::filesystem::exists(projectOld)) {
                        std::filesystem::rename(projectOld, projectNew);
                    }

                    addDebugMessage("Renamed asset: " + old.string() + " -> " + newPath.string() + "\n");
                }
                else {
                    addDebugMessage("Failed to rename: file already exists.\n");
                }
            }

            m_AssetBrowserState.fullPathToRename.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel")) {
            m_AssetBrowserState.fullPathToRename.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void UI::assetReplacePopup() {
    if (m_AssetBrowserState.showReplacePopup) {
        ImGui::OpenPopup("Replace Asset");
        m_AssetBrowserState.showReplacePopup = false;
    }

    if (ImGui::BeginPopupModal("Replace Asset", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Select a new file to replace the asset:");

        ImGui::InputText("File Path", m_AssetBrowserState.replacePathBuffer, sizeof(m_AssetBrowserState.replacePathBuffer));

        if (ImGui::Button("Replace")) {
            std::filesystem::path old = m_AssetBrowserState.fullPathToReplace;
            std::filesystem::path newPath = m_AssetBrowserState.replacePathBuffer;

            if (std::filesystem::exists(newPath)) {
                std::filesystem::copy_file(newPath, old, std::filesystem::copy_options::overwrite_existing);
                assetsChangedFlag = true;
                addDebugMessage("Replaced asset: " + old.string() + " with " + newPath.string() + "\n");
            }
            else {
                addDebugMessage("Replacement file does not exist.\n");
            }

            m_AssetBrowserState.fullPathToReplace.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            m_AssetBrowserState.fullPathToReplace.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void UI::editorCameraControls() {
	// edit camera position and zoom for editor camera
    if (ImGui::Begin("Editor Camera")) {
        // change the centre pos of the camera
        ImGui::DragFloat3("Position", glm::value_ptr(renderer::editorCam.campos), 0.1f);

		// change zoom level of the camera
        ImGui::DragFloat("Zoom", &renderer::editorCam.zoom, 0.1f, renderer::editorCam.minZoom, renderer::editorCam.maxZoom + renderer::editorCam.zoom);

		// change zoom factor (how fast zoom in/out)
        ImGui::DragFloat("Zoom Factor", &renderer::editorCam.zoomFactor, 0.01f, renderer::editorCam.minZoom, renderer::editorCam.maxZoom + renderer::editorCam.zoom);

        // set min zoom range (able to zoom in closer than using mouse)
        ImGui::DragFloat("Min Zoom", &renderer::editorCam.minZoom, 0.1f, renderer::editorCam.minZoom, renderer::editorCam.maxZoom + renderer::editorCam.zoom);

		// set max zoom range (able to zoom out further than using mouse)
        ImGui::DragFloat("Max Zoom", &renderer::editorCam.maxZoom, 0.1f, renderer::editorCam.minZoom, renderer::editorCam.maxZoom + renderer::editorCam.zoom);

        ImGui::Separator();
        if (ImGui::Button("Reset Camera")) {
            renderer::editorCam.campos = glm::vec3(0.f);
            renderer::editorCam.zoom = 10.f;
        }
    }
    ImGui::End();

}

void UI::sceneWindow(GLuint texture, float aspectRatio) {

    ImGui::Begin("Scene", nullptr);

    ImVec2 avail = ImGui::GetContentRegionAvail();
    ImVec2 size = avail;

    // calculate how much to scale based on aspect ratio
    float availAspect = avail.x / avail.y;
    if (availAspect > aspectRatio) size.x = avail.y * aspectRatio;
    else size.y = avail.x / aspectRatio;

    // center in the available space
    ImVec2 cursor = ImGui::GetCursorPos();
    ImVec2 offset = { (avail.x - size.x) * 0.5f, (avail.y - size.y) * 0.5f };
    if (offset.x < 0) offset.x = 0;
    if (offset.y < 0) offset.y = 0;
    ImGui::SetCursorPos({ cursor.x + offset.x, cursor.y + offset.y });


    // draw the scene in a child window to prevent moving/resizing this child viewport
    ImGui::BeginChild("SceneViewport", size, false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    ImGui::Image((void*)(intptr_t)texture, size, ImVec2(0, 1), ImVec2(1, 0));

    if (ImGui::IsWindowHovered())
    {
        // Pan with middle mouse button
        if (InputHandler::isMouseDragging(GLFW_MOUSE_BUTTON_MIDDLE))
        {
            renderer::editorCam.pan(InputHandler::getMouseDeltaWorldInViewport(m_SceneWindowState.sceneSize));
        }

        if (InputHandler::getMouseScroll() != 0.f)
        {
            renderer::editorCam.zoomInOut(InputHandler::getMouseScroll());
        }

    }

    ImGui::EndChild();


    m_SceneWindowState.scenePos = ImGui::GetItemRectMin();
    m_SceneWindowState.sceneSize = ImGui::GetItemRectSize();

    ImGui::End();
}

/*
Future implementation (jic i forget)
- Add snapping to grid or axis (e.g. hold Shift for 1-unit steps)

- Highlight selected object while dragging

- Undo/redo
*/
void UI::objPicking(GameObjectManager& manager) {
    if (!InputHandler::isMouseLeftClicked()) return;

    Vector2D mousePos = InputHandler::getMousePositionInImGuiViewport(m_SceneWindowState.scenePos, m_SceneWindowState.sceneSize);

    std::vector<GameObject*> gameObjects;
    manager.getAllGameObjects(gameObjects);

    GameObject* selected = nullptr;

    for (auto* obj : gameObjects) {
        // dont let user click and select platform
        if (obj->getObjectName() == "platform") continue;

        Transform* transform = obj->getComponent<Transform>();
        if (!transform) continue;

        Collision::AABB aabb = Collision::getObjectAABB(transform);
        Collision::Circle mousePoint(mousePos, 0.2f);

        // check if mouse point intersects with the game obj
        if (Collision::CollisionIntersection_CircleAABB_Static(mousePoint, aabb)) {
            selected = obj;
        }
    }

    // if something is selected, update selection state
    if (selected) {
        m_SelectionState.selectedObject = selected;
        m_SelectionState.draggedObject = selected;

        auto iterator = std::find(gameObjects.begin(), gameObjects.end(), selected);
        if (iterator != gameObjects.end()) {
            m_SelectionState.selectedIndex = static_cast<int>(std::distance(gameObjects.begin(), iterator));
        }

        addDebugMessage(selected->getObjectName() + " selected");
    }
    else {
        // if mouse click somewhere else, deselect current dragged object
        m_SelectionState.draggedObject = nullptr;
    }
}

void UI::objDragging() const {
    // early return if no obj is selected
    GameObject* obj = m_SelectionState.draggedObject;
    if (!obj) return;

    // early return if mouse is not dragging
    if (!InputHandler::isMouseDragging()) return;

    Transform* transform = obj->getComponent<Transform>();
    if (!transform) return;

    // change current object position based on mouse delta
    Vector2D mousePos = InputHandler::getMouseDeltaWorldInViewport(m_SceneWindowState.sceneSize);
    transform->x += mousePos.x;
    transform->y += mousePos.y;
}