/* Start Header ************************************************************************/
/*!
\file       assetBrowser.cpp
\author     Tan Chee Qing, cheeqing.tan, 2401486
            - 100%
\par		cheeqing.tan@digipen.edu
\date       November, 10th, 2025
\brief      Definitions of the functions for rendering of Asset Browser and hot loading of
            assets from the assets folder. Include the actions that be done on a asset 
            (rename/delete/show in explorer/replace).

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#ifdef _DEBUG
#include "Systems.h"
#include "Editor/editorManager.h"

AssetBrowser::AssetBrowser(Editor::AssetBrowserState& Astate, Editor::ObjSelectionState& Ostate, Editor::MenuBarState& Mstate) :
    m_AssetBrowserState(Astate),
    m_objSelectionState(Ostate),
    m_menuBarState(Mstate),
    m_assets(Editor::assetBrowserState.assets)
{}

void AssetBrowser::render() {
    m_AssetBrowserState.hoverFolder = "";

    ImGui::Begin("Assets");

    // refresh to update assets from the actual asset folder
    // not really needed since we have auto refresh, but still good to have
    if (ImGui::Button("Refresh")) {
        EditorManager::assetChanged();
    }


    /* ------------------ Auto update if changes in project directory ---------------------*/
    // check top-level folder got any updates (the asset folder)
    static float timeSinceLastCheck = 0.f;
    timeSinceLastCheck += ImGui::GetIO().DeltaTime;

	// check every 5 seconds
    if (timeSinceLastCheck >= 5.f) {
        timeSinceLastCheck = 0.f;
        std::filesystem::file_time_type currentTopLevel = std::filesystem::last_write_time(m_AssetBrowserState.currentFolder);
        bool needsRefresh = false;

        if (currentTopLevel != m_AssetBrowserState.lastTopLevelRefresh) {
            m_AssetBrowserState.lastTopLevelRefresh = currentTopLevel;
            needsRefresh = true;
        }

        // check subfolders recursively (only if assetsChangedFlag is false)
        std::filesystem::file_time_type newestTime = m_AssetBrowserState.lastRefresh;

        for (const auto& entry : std::filesystem::recursive_directory_iterator(m_AssetBrowserState.currentFolder)) {
            auto t = std::filesystem::last_write_time(entry);
            if (t > newestTime) {
                newestTime = t;
                needsRefresh = true;
                break;
            }
        }

        // update and reload if necessary
        if (needsRefresh) {
            m_AssetBrowserState.lastRefresh = newestTime;
            EditorManager::assetChanged();
        }
    }
    /* ------------------ End ---------------------*/


    ImGui::Separator();

    if (!m_AssetBrowserState.currentViewFolder.empty()) {
        if (ImGui::Button("< Back")) {
            std::filesystem::path current(m_AssetBrowserState.currentViewFolder);
            std::string parent = current.parent_path().string();

            m_AssetBrowserState.currentViewFolder = parent;

            EditorManager::assetChanged();
        }

        // if inside a folder and drag and drop a file, add it into the folder
        m_AssetBrowserState.hoverFolder = m_AssetBrowserState.currentViewFolder;

        ImGui::SameLine();
        ImGui::Text("Folder: %s", m_AssetBrowserState.currentViewFolder.c_str());
        ImGui::Separator();

        /*if (!m_AssetBrowserState.currentViewSubFolder.empty()) {
            ImGui::SameLine();
            ImGui::Text("/ %s", m_AssetBrowserState.currentViewSubFolder.c_str());
        }*/

        ImGui::Separator();

        for (const std::string& subFolder : m_AssetBrowserState.currentViewSubFolders) {
            ImGui::PushID(("subfolder_" + subFolder).c_str());
            ImGui::BeginGroup();
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(100, 40));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.25f, 0.55f, 1.0f)); // blue tint to distinguish

            // subfolder button
            if (ImGui::Button(subFolder.c_str(), ImVec2(m_AssetBrowserState.iconSize, m_AssetBrowserState.iconSize))) {
                m_AssetBrowserState.currentViewFolder = m_AssetBrowserState.currentViewFolder + "/" + subFolder;

                EditorManager::assetChanged();
            }

            ImGui::PopStyleColor();
            ImGui::PopStyleVar();

            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + m_AssetBrowserState.iconSize);
            ImGui::Text("%s", subFolder.c_str());
            ImGui::PopTextWrapPos();
            ImGui::EndGroup();

            // RIGHT CLICK subfolder to rename / delete
            if (ImGui::BeginPopupContextItem(("subfolderCtx_" + subFolder).c_str())) {
                if (ImGui::MenuItem("Rename")) {
                    m_AssetBrowserState.folderToRename =
                        (std::filesystem::path(m_AssetBrowserState.currentFolder)
                            / m_AssetBrowserState.currentViewFolder / subFolder).string();

                    std::strncpy(m_AssetBrowserState.renameFolderBuffer, subFolder.c_str(),
                        sizeof(m_AssetBrowserState.renameFolderBuffer));

                    m_AssetBrowserState.showRenameSubfolderPopup = true;
                }

                if (ImGui::MenuItem("Delete")) {
                    m_AssetBrowserState.folderToDelete =
                        (std::filesystem::path(m_AssetBrowserState.currentFolder)
                            / m_AssetBrowserState.currentViewFolder / subFolder).string();

                    m_AssetBrowserState.showDeleteSubfolderPopup = true;
                }

                ImGui::EndPopup();
            }

            // ---- Drop assets INTO this subfolder button ----
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET")) {
                    std::string srcPath(static_cast<const char*>(payload->Data));
                    std::filesystem::path src(srcPath);
                    std::filesystem::path destDir =
                        std::filesystem::path(m_AssetBrowserState.currentFolder)
                        / m_AssetBrowserState.currentViewFolder
                        / subFolder;
                    std::filesystem::path dest = destDir / src.filename();

                    if (!std::filesystem::exists(dest)) {
                        try {
                            std::filesystem::rename(src, dest);  // moves the file

                            if (src.extension() == ".json") {
                                std::string prefabID = PrefabManager::Instance().findPrefabIDByFilename(toRegistryPath(src.string()));

                                if (!prefabID.empty()) {
                                    // build new path relative to assets folder: "/Prefab/subfolder/filename.json"
                                    std::string newRelativePath = toRegistryPath(dest.string());

                                    // update in prefab registry
                                    PrefabManager::Instance().setPrefabPath(prefabID, newRelativePath);
                                }
                            }

                            // Also mirror move in SOURCE_DIR_R (project folder) if needed
                            std::string relative;
                            size_t pos = srcPath.find("assets/");
                            if (pos != std::string::npos) relative = srcPath.substr(pos + strlen("assets/"));
                            if (!relative.empty()) {
                                std::filesystem::path projectSrc = std::filesystem::path(std::string(SOURCE_DIR_R)) / relative;
                                std::filesystem::path projectDest = std::filesystem::path(std::string(SOURCE_DIR_R))
                                    / m_AssetBrowserState.currentViewFolder / subFolder / src.filename();

                                if (std::filesystem::exists(projectSrc))
                                    std::filesystem::rename(projectSrc, projectDest);
                            }

                            EditorManager::assetChanged();
                            DebugLog::addMessage("Moved " + src.filename().string() + " into subfolder: " + subFolder);
                        }
                        catch (const std::filesystem::filesystem_error& e) {
                            DebugLog::addMessage("Move failed: " + std::string(e.what()));
                        }
                    }
                }
                ImGui::EndDragDropTarget();
            }

            float windowVisibleX2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

            float lastButtonX2 = ImGui::GetItemRectMax().x;
            float nextButtonX2 = lastButtonX2 + m_AssetBrowserState.iconSize;
            if (nextButtonX2 < windowVisibleX2) ImGui::SameLine();

            ImGui::PopID();
        }
    }

    float windowVisibleX2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

    // show folders & files directly under Assets folder
    if (m_AssetBrowserState.currentViewFolder.empty()) {
        for (const std::string& folder : m_AssetBrowserState.subFolders) {
            ImGui::PushID(folder.c_str());

            ImGui::BeginGroup();
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(100, 40));

            // folder button
            if (ImGui::Button(folder.c_str(), ImVec2(m_AssetBrowserState.iconSize, m_AssetBrowserState.iconSize))) {
                m_AssetBrowserState.currentViewFolder = folder;

                EditorManager::assetChanged(); // refresh subfolder
            }

            // see if mouse is on which folder (for dropping of files from file explorer)
            if (ImGui::IsItemHovered()) {
                m_AssetBrowserState.hoverFolder = folder;
            }

            ImGui::PopStyleVar();

            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + m_AssetBrowserState.iconSize);
            ImGui::Text("%s", folder.c_str());
            ImGui::PopTextWrapPos();

            ImGui::EndGroup();

            float lastButtonX2 = ImGui::GetItemRectMax().x;
            float nextButtonX2 = lastButtonX2 + m_AssetBrowserState.iconSize;
            if (nextButtonX2 < windowVisibleX2) ImGui::SameLine();

            ImGui::PopID();
        }
    }

    int index{};
    for (const Editor::Asset& asset : m_assets) {
        // New fixed code:
        if (m_AssetBrowserState.currentViewFolder.empty()) continue;

        std::filesystem::path expectedDir =
            std::filesystem::path(m_AssetBrowserState.currentFolder) / m_AssetBrowserState.currentViewFolder;

        // Misc special case: files sitting directly in assets root
        std::string assetFolderName = std::filesystem::path(asset.folder).filename().string();
        if (m_AssetBrowserState.currentViewFolder == "Misc" && assetFolderName == "assets") {
            // show it
        }
        else if (std::filesystem::path(asset.folder) != expectedDir) {
            continue;
        }

        std::string fullPath = asset.folder + "/" + asset.name;

        ImGui::PushID(index++);

        // highlight if the asset is the current scene
        if (asset.name == Editor::sceneState.currentSceneName) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.f, 0.4f, 0.7f, 1.0f));
            /*ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.6f, 0.9f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.4f, 0.7f, 1.0f));*/
        }

        //ImGui::PopStyleVar();
        ImGui::BeginGroup();

        // check if its image
        std::string extension;
        size_t dotPos = asset.name.find_last_of('.');
        if (dotPos != std::string::npos) extension = asset.name.substr(dotPos + 1);

        bool isImage = (extension == "png" || extension == "jpg" || extension == "jpeg");

        if (isImage) {
            // load texture to show image preview
            TextureData texData = ResourceManager::getInstance().getTexture(asset.path);

            if (texData.id != 0) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));

                ImGui::ImageButton(("##img" + asset.name).c_str(),
                    (ImTextureID)(intptr_t)texData.id,
                    ImVec2(m_AssetBrowserState.iconSize, m_AssetBrowserState.iconSize),
                    ImVec2(0, 1), ImVec2(1, 0));

                ImGui::PopStyleVar();
                ImGui::PopStyleColor();
            }
            else {
                // fallback to button if texture fails
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(100, 40));
                ImGui::Button(asset.name.c_str(), ImVec2(m_AssetBrowserState.iconSize, m_AssetBrowserState.iconSize));
                ImGui::PopStyleVar();
            }
        }
        else {
            // if not image then use button
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(100, 40));
            ImGui::Button(asset.name.c_str(), ImVec2(m_AssetBrowserState.iconSize, m_AssetBrowserState.iconSize));
            ImGui::PopStyleVar();
        }

        // pop style for the highlight current scene
        if (asset.name == Editor::sceneState.currentSceneName) {
            ImGui::PopStyleColor();
        }

        // RIGHT CLICK asset and show context menu
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

            // open popup window and ask whether to delete this current asset (delete not for prefab YET)
            //if (assetFolderName != "Prefab") {
                if (ImGui::MenuItem("Delete Asset")) {
                    m_AssetBrowserState.fullPathToDelete = fullPath;
                    m_AssetBrowserState.showDeletePopup = true;
                }
            //}

            // open popup window and rename this current asset
            if (ImGui::MenuItem("Rename Asset")) {
                m_AssetBrowserState.fullPathToRename = fullPath;
                m_AssetBrowserState.showRenamePopup = true;

                // pre-fill the current name
                std::strncpy(m_AssetBrowserState.newNameBuffer, asset.name.c_str(), sizeof(m_AssetBrowserState.newNameBuffer));
            }

            // open popup window and replace this current asset (not for prefab)
            if (assetFolderName != "Prefab" && assetFolderName != "Scene") {
                if (ImGui::MenuItem("Replace Asset")) {
                    m_AssetBrowserState.fullPathToReplace = fullPath;
                    m_AssetBrowserState.showReplacePopup = true;
                    m_AssetBrowserState.replacePathBuffer[0] = '\0';
                }
            }

            ImGui::EndPopup();
        }

        // DOUBLE click logic
        if (ImGui::IsItemHovered() && InputHandler::isMouseLeftDoubleClicked()) {
            // if its a scene file, load the scene
            if (assetFolderName == "Scene") {
                m_menuBarState.sceneToLoad = asset.name;
                m_menuBarState.showLoadScenePopup = true;
            }
            // if its a logic, open
            else if (assetFolderName == "Logic") {
                Editor::visualLogicState.targetObj = nullptr;
                VisualLogicEditor::Instance().loadLogic(asset.name);
            }
            // if its a lua, open in VSC
            else if (extension == "lua") {
                std::filesystem::path absPath = std::filesystem::absolute(fullPath);
                std::string absPathStr = absPath.string();
                std::replace(absPathStr.begin(), absPathStr.end(), '/', '\\');

                std::string command = "code \"" + absPathStr + "\"";
                system(command.c_str());
                DebugLog::addMessage("Opening " + asset.name + " in VS Code.\n");
            }
            // if its a prefab, open in inspector
            else {
                // if its a prefab, convert to temp game obj for editing in inspector
            //else if (assetFolderName == "Prefab") {
                bool isInPrefabFolder = false;
                std::filesystem::path folderPath(asset.folder);
                for (const auto& part : folderPath) {
                    if (part.string() == "Prefab") {
                        isInPrefabFolder = true;
                        break;
                    }
                }

                if (isInPrefabFolder) {

                    // get ID of the double clicked prefab by its filename
                    std::string prefabID = PrefabManager::Instance().findPrefabIDByFilename(toRegistryPath(asset.path));
                    if (prefabID.empty()) {
                        DebugLog::addMessage("Cannot find prefab ID for " + asset.name + ".\n");
                        return;
                    }

                    // set the selected prefab to double clicked prefab, later will show in inspector
                    std::unique_ptr<GameObject> tempPrefab = PrefabManager::Instance().createTempPrefabObj(prefabID);
                    if (tempPrefab) {
                        m_objSelectionState.selectedObject = nullptr;
                        m_objSelectionState.selectedIndex = -1;
                        m_objSelectionState.selectedPrefab = std::move(tempPrefab);

                        DebugLog::addMessage("Open Prefab " + m_objSelectionState.selectedPrefab->getObjectName() + ".\n");
                    }
                    else {
                        DebugLog::addMessage("Failed to open prefab " + prefabID + " object for editing.\n");
                    }

                }
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


        /* ----------- Drag and Drop Logic ----------- */

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            // drag a texture to inspector/hierarchy window
            ImGui::SetDragDropPayload("ASSET", asset.path.c_str(), asset.path.size() + 1);
            ImGui::Text("%s", asset.name.c_str());
            ImGui::EndDragDropSource();
        }

        /* ---------- END ---------- */

        ImGui::PopID();
    }

    if (!m_AssetBrowserState.currentViewFolder.empty()) {
        // Only trigger context menu on empty space (not on an item)
        if (ImGui::BeginPopupContextWindow("##EmptySpaceContext",
            ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
            if (ImGui::MenuItem("Add Folder")) {
                m_AssetBrowserState.newSubfolderNameBuffer[0] = '\0';
                m_AssetBrowserState.showCreateSubfolderPopup = true;
            }

            ImGui::EndPopup();
        }
    }


    ImGui::End();

    /* -------- Asset pop up modals call -------- */
    assetDeletePopup();
    assetRenamePopup();
    assetReplacePopup();
    createSubfolderPopup();
    renameSubfolderPopup();
    deleteSubfolderPopup();
    /* -------- END -------- */
}

void AssetBrowser::loadAssetsFromDirectory() {
    m_assets.clear(); // clear existing assets before loading new ones
    m_AssetBrowserState.subFolders.clear();

    for (const auto& entry : std::filesystem::directory_iterator(m_AssetBrowserState.currentFolder)) {
        if (entry.is_directory()) {
            std::string folderName = entry.path().filename().string();
            if (folderName == "resources") continue;
            m_AssetBrowserState.subFolders.push_back(folderName);
        }
    }

    // for all files directly in Assets
    m_AssetBrowserState.subFolders.push_back("Misc");

    // iterate over all files and folders in this directory and push all files to m_assets (flat list)
    for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(m_AssetBrowserState.currentFolder)) {
        if (entry.is_regular_file()) {
            Editor::Asset asset;
            std::string filename = entry.path().filename().string();
            std::string extension = entry.path().extension().string();

            // skip unwanted files
            if (filename == Editor::sceneState.tempSceneName) continue;
            if (filename == "prefab_registry.json") continue;

            asset.name = std::move(filename);
            asset.path = entry.path().string();
            asset.folder = entry.path().parent_path().string(); // folder in assets that contains this file, for future filtering

            ///* ---- Sample for the asset struct ---- */
            // filename: control_scene.json
            // folder: assets/Scene ->asset+immediate parent folder
            // path: assets/Scene\control_scene.json
            ///* ---- END ---- */

            /*std::cout << "Asset Name: " << asset.name <<
                "\n Asset Path: " << asset.path <<
                "\n Asset Folder: " << asset.folder << std::endl;*/
            m_assets.push_back(asset);
        }
    }

    // populate subfolders for the currently viewed folder
    m_AssetBrowserState.currentViewSubFolders.clear();
    if (!m_AssetBrowserState.currentViewFolder.empty()) {
        std::filesystem::path viewedPath =
            std::filesystem::path(m_AssetBrowserState.currentFolder) / m_AssetBrowserState.currentViewFolder;

        if (std::filesystem::exists(viewedPath) && std::filesystem::is_directory(viewedPath)) {
            for (const auto& entry : std::filesystem::directory_iterator(viewedPath)) {
                if (entry.is_directory()) {
                    m_AssetBrowserState.currentViewSubFolders.push_back(
                        entry.path().filename().string());
                }
            }
        }
    }

    //DebugLog::addMessage("Assets loaded from " + m_AssetBrowserState.currentFolder);
}

// Add this as a private static helper in AssetBrowser or a free function
std::string AssetBrowser::toRegistryPath(const std::string& assetPath) {
    std::string p = assetPath;
    std::replace(p.begin(), p.end(), '\\', '/');
    size_t pos = p.find("/Prefab/");
    if (pos != std::string::npos) return p.substr(pos);
    pos = p.find("Prefab/");
    if (pos != std::string::npos) return "/" + p.substr(pos);
    return p;
}

void AssetBrowser::assetDeletePopup() {
    // open popup if flagged
    if (m_AssetBrowserState.showDeletePopup) {
        // Check if it's a prefab being deleted
        std::filesystem::path pathToDelete(m_AssetBrowserState.fullPathToDelete);
        std::string folder = pathToDelete.parent_path().filename().string();

        /*if (folder == "Prefab") {
            std::string prefabID = PrefabManager::Instance().findPrefabIDByFilename(pathToDelete.filename().string());*/
        bool isInPrefabFolder = false;
        for (const auto& part : pathToDelete) {
            if (part.string() == "Prefab") { isInPrefabFolder = true; break; }
        }

        if (isInPrefabFolder) {
            std::string prefabID = PrefabManager::Instance().findPrefabIDByFilename(toRegistryPath(pathToDelete.string()));

            if (!prefabID.empty()) {
                std::vector<PrefabUsageInfo> usageInfo;

                if (PrefabManager::Instance().isPrefabUsedInAnyScene(prefabID, usageInfo)) {
                    // store usage info for the popup
                    m_AssetBrowserState.prefabUsageInfo = usageInfo;

                    ImGui::OpenPopup("Cannot Delete Prefab");
                    m_AssetBrowserState.showDeletePopup = false;
                    return;
                }
            }
        }
        else if (folder == "Logic") {
			if (pathToDelete.filename().string() == Editor::visualLogicState.currentGraphName) {
				ImGui::OpenPopup("Cannot Delete Logic");
				m_AssetBrowserState.showDeletePopup = false;
				return;
			}
        }

        ImGui::OpenPopup("Confirm Delete Asset");
        m_AssetBrowserState.showDeletePopup = false;
    }


    /*---------------- Delete Confirmation ----------------*/
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Confirm Delete Asset", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to delete this asset?");

        if (ImGui::Button("Delete") || InputHandler::isKeyTriggered(GLFW_KEY_ENTER)) {

            if (!m_AssetBrowserState.fullPathToDelete.empty() &&
                std::filesystem::exists(m_AssetBrowserState.fullPathToDelete)) {

                bool deletionSuccess = true;

                // delete from prefab registry FIRST (before files) if its prefab
                std::string prefabID = PrefabManager::Instance().findPrefabIDByFilename(toRegistryPath(m_AssetBrowserState.fullPathToDelete));
                if (!prefabID.empty()) {
                    if (!PrefabManager::Instance().deletePrefab(prefabID)) {
                        DebugLog::addMessage("Failed to delete prefab from registry: " + prefabID);
                        deletionSuccess = false;
                    }
                }

                // delete files only if registry deletion succeeded
                if (deletionSuccess) {
                    try {
                        // delete from assets folder
                        if (!std::filesystem::remove(m_AssetBrowserState.fullPathToDelete)) {
                            DebugLog::addMessage("Failed to delete asset file");
                            deletionSuccess = false;
                        }

                        // extract relative path first
                        std::string relativePath;
                        size_t pos = m_AssetBrowserState.fullPathToDelete.find("assets/");
                        if (pos != std::string::npos) {
                            relativePath = m_AssetBrowserState.fullPathToDelete.substr(pos + strlen("assets/"));
                        }

                        // delete from project folder
                        if (!relativePath.empty()) {
                            std::filesystem::path projectPath = std::string(SOURCE_DIR_R) + "/" + relativePath;
                            if (std::filesystem::exists(projectPath)) {
                                std::filesystem::remove(projectPath);
                            }
                        }

                        if (deletionSuccess) {
                            EditorManager::assetChanged();
                            DebugLog::addMessage("Deleted asset: " + m_AssetBrowserState.fullPathToDelete);
                        }
                    }
                    catch (const std::filesystem::filesystem_error& e) {
                        DebugLog::addMessage("Filesystem error during deletion: " + std::string(e.what()));
                        deletionSuccess = false;
                    }
                }
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

    /*---------------- Cannot Delete Prefab In Use ----------------*/
   /* ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));*/

    if (ImGui::BeginPopupModal("Cannot Delete Prefab", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Cannot delete Prefab!");
        ImGui::Text("This prefab is being used in the following scenes:");
        ImGui::Separator();

        // Show usage grouped by scene
        for (const auto& sceneInfo : m_AssetBrowserState.prefabUsageInfo) {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%s", sceneInfo.sceneName.c_str());

            for (const auto& objName : sceneInfo.objectNames) {
                ImGui::BulletText("%s", objName.c_str());
            }

            ImGui::Spacing();
        }

        ImGui::Separator();
        ImGui::TextWrapped("Please remove these objects or unpack prefab before deleting.");

        if (ImGui::Button("OK", ImVec2(120, 0)) || InputHandler::isKeyTriggered(GLFW_KEY_ENTER)) {
            m_AssetBrowserState.fullPathToDelete.clear();
            m_AssetBrowserState.prefabUsageInfo.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
    /*-------------------- END -------------------*/


    /*---------------- Cannot Delete Current Logic ----------------*/
   /* ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));*/

    if (ImGui::BeginPopupModal("Cannot Delete Logic", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Cannot delete Logic!");
        ImGui::Text("This logic is currently opened in editor.");
        ImGui::Separator();

        ImGui::Separator();
        ImGui::TextWrapped("Please close the Visual Logic Editor before proceed to delete.");

        if (ImGui::Button("OK", ImVec2(120, 0)) || InputHandler::isKeyTriggered(GLFW_KEY_ENTER)) {
            m_AssetBrowserState.fullPathToDelete.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
    /*-------------------- END -------------------*/


    /*---------------- Cannot delete current loaded scene ----------------*/
    if (!m_AssetBrowserState.fullPathToDelete.empty() &&
        Editor::sceneState.currentSceneName == std::filesystem::path(m_AssetBrowserState.fullPathToDelete).filename().string())
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
}

void AssetBrowser::assetRenamePopup() {
    // open popup if flagged
    if (m_AssetBrowserState.showRenamePopup) {
        ImGui::OpenPopup("Rename Asset");
        m_AssetBrowserState.showRenamePopup = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Rename Asset", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter new name for the asset");

        ImGui::InputText("##newName", m_AssetBrowserState.newNameBuffer, sizeof(m_AssetBrowserState.newNameBuffer));

        if (ImGui::Button("Rename") || InputHandler::isKeyTriggered(GLFW_KEY_ENTER)) {
            if (!m_AssetBrowserState.fullPathToRename.empty()) {
                std::filesystem::path old = m_AssetBrowserState.fullPathToRename;
                std::filesystem::path newPath = old.parent_path() / m_AssetBrowserState.newNameBuffer;
                // Debug/shaders/Prefab\\platform.json

                // keep the extension if not provided
                if (newPath.extension().empty())
                    newPath.replace_extension(old.extension());

                if (!std::filesystem::exists(newPath)) {
                    // if renamed asset is current scene, rename current scene name as well
                    if (Editor::sceneState.currentSceneName == old.filename().string()) {
                        Editor::sceneState.currentSceneName = newPath.filename().string();
                    }

                    std::filesystem::rename(old, newPath);
                    EditorManager::assetChanged(); // reload assets to reflect new name

                    // change in the project folder as well
                    std::filesystem::path sourceDir(SOURCE_DIR_R);

                    // split the "assets/" part to get relative path
                    std::string relative;
                    size_t pos = old.string().find("assets/");
                    if (pos != std::string::npos) {
                        relative = old.string().substr(pos + strlen("assets/"));
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


                    /* -------- Changing Prefab Name + Registry Entry -------- */

                    // get prefabid by the path
                    std::string prefabID = PrefabManager::Instance().findPrefabIDByFilename(toRegistryPath(old.string()));
                    if (!prefabID.empty()) {
                        // change prefab name and save
                        PrefabManager::Instance().setPrefabPath(prefabID, toRegistryPath(newPath.string()));
                    }

                    /* ------- END ------- */

                    DebugLog::addMessage("Renamed asset: " + old.string() + " -> " + newPath.string() + "\n");
                }
                else {
                    DebugLog::addMessage("Failed to rename: file already exists.\n");
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

void AssetBrowser::assetReplacePopup() {
    if (m_AssetBrowserState.showReplacePopup) {
        ImGui::OpenPopup("Replace Asset");
        m_AssetBrowserState.showReplacePopup = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Replace Asset", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Select a new file to replace the asset:\n");

        ImGui::InputText("File Path", m_AssetBrowserState.replacePathBuffer, sizeof(m_AssetBrowserState.replacePathBuffer));

        ImGui::Text("Please enter the absolute path without \"\"");

        if (ImGui::Button("Replace") || InputHandler::isKeyTriggered(GLFW_KEY_ENTER)) {
            std::filesystem::path old = m_AssetBrowserState.fullPathToReplace;
            std::filesystem::path newPath = m_AssetBrowserState.replacePathBuffer;

            if (std::filesystem::exists(newPath)) {
                std::filesystem::copy_file(newPath, old, std::filesystem::copy_options::overwrite_existing);
                EditorManager::assetChanged();
                DebugLog::addMessage("Replaced asset: " + old.string() + " with " + newPath.string() + "\n");
            }
            else {
                DebugLog::addMessage("Replacement file does not exist.\n");
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

void AssetBrowser::createSubfolderPopup() {
    if (m_AssetBrowserState.showCreateSubfolderPopup) {
        ImGui::OpenPopup("Add Folder");
        m_AssetBrowserState.showCreateSubfolderPopup = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Add Folder", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter folder name:");
        ImGui::InputText("##folderName", m_AssetBrowserState.newSubfolderNameBuffer,
            sizeof(m_AssetBrowserState.newSubfolderNameBuffer));

        if (ImGui::Button("Create") || InputHandler::isKeyTriggered(GLFW_KEY_ENTER)) {
            std::string folderName = m_AssetBrowserState.newSubfolderNameBuffer;
            if (!folderName.empty()) {
                // Create in runtime assets folder
                std::filesystem::path newDir =
                    std::filesystem::path(m_AssetBrowserState.currentFolder)
                    / m_AssetBrowserState.currentViewFolder
                    / folderName;

                // Create in project source folder too
                std::filesystem::path newDirSource =
                    std::filesystem::path(std::string(SOURCE_DIR_R))
                    / m_AssetBrowserState.currentViewFolder
                    / folderName;

                try {
                    std::filesystem::create_directories(newDir);
                    std::filesystem::create_directories(newDirSource);
                    EditorManager::assetChanged();
                    DebugLog::addMessage("Created folder: " + newDir.string());
                }
                catch (const std::filesystem::filesystem_error& e) {
                    DebugLog::addMessage("Failed to create folder: " + std::string(e.what()));
                }
            }
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void AssetBrowser::renameSubfolderPopup() {
    if (m_AssetBrowserState.showRenameSubfolderPopup) {
        ImGui::OpenPopup("Rename Folder");
        m_AssetBrowserState.showRenameSubfolderPopup = false;
    }

    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Rename Folder", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter new folder name:");
        ImGui::InputText("##renameFolderInput", m_AssetBrowserState.renameFolderBuffer,
            sizeof(m_AssetBrowserState.renameFolderBuffer));

        if (ImGui::Button("Rename") || InputHandler::isKeyTriggered(GLFW_KEY_ENTER)) {
            std::string newName = m_AssetBrowserState.renameFolderBuffer;
            if (!newName.empty() && !m_AssetBrowserState.folderToRename.empty()) {
                std::filesystem::path oldPath(m_AssetBrowserState.folderToRename);
                std::filesystem::path newPath = oldPath.parent_path() / newName;

                if (!std::filesystem::exists(newPath)) {
                    try {
                        // rename in runtime assets
                        std::filesystem::rename(oldPath, newPath);

                        // mirror rename in source project folder
                        std::string relative;
                        size_t pos = m_AssetBrowserState.folderToRename.find("assets/");
                        if (pos != std::string::npos)
                            relative = m_AssetBrowserState.folderToRename.substr(pos + strlen("assets/"));
                        if (!relative.empty()) {
                            std::filesystem::path projectOld = std::filesystem::path(std::string(SOURCE_DIR_R)) / relative;
                            std::filesystem::path projectNew = projectOld.parent_path() / newName;
                            if (std::filesystem::exists(projectOld))
                                std::filesystem::rename(projectOld, projectNew);
                        }

                        EditorManager::assetChanged();
                        DebugLog::addMessage("Renamed folder: " + oldPath.string() + " -> " + newPath.string());
                    }
                    catch (const std::filesystem::filesystem_error& e) {
                        DebugLog::addMessage("Failed to rename folder: " + std::string(e.what()));
                    }
                }
                else {
                    DebugLog::addMessage("Folder with that name already exists.");
                }
            }
            m_AssetBrowserState.folderToRename.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            m_AssetBrowserState.folderToRename.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void AssetBrowser::deleteSubfolderPopup() {
    if (m_AssetBrowserState.showDeleteSubfolderPopup) {
        ImGui::OpenPopup("Confirm Delete Folder");
        m_AssetBrowserState.showDeleteSubfolderPopup = false;
    }

    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Confirm Delete Folder", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Delete this folder and ALL its contents?");
        ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "%s", m_AssetBrowserState.folderToDelete.c_str());
        ImGui::Separator();

        if (ImGui::Button("Delete") || InputHandler::isKeyTriggered(GLFW_KEY_ENTER)) {
            if (!m_AssetBrowserState.folderToDelete.empty() &&
                std::filesystem::exists(m_AssetBrowserState.folderToDelete)) {
                try {
                    // delete from runtime assets
                    std::filesystem::remove_all(m_AssetBrowserState.folderToDelete);

                    // mirror delete in source project folder
                    std::string relative;
                    size_t pos = m_AssetBrowserState.folderToDelete.find("assets/");
                    if (pos != std::string::npos)
                        relative = m_AssetBrowserState.folderToDelete.substr(pos + strlen("assets/"));
                    if (!relative.empty()) {
                        std::filesystem::path projectPath = std::filesystem::path(std::string(SOURCE_DIR_R)) / relative;
                        if (std::filesystem::exists(projectPath))
                            std::filesystem::remove_all(projectPath);
                    }

                    EditorManager::assetChanged();
                    DebugLog::addMessage("Deleted folder: " + m_AssetBrowserState.folderToDelete);
                }
                catch (const std::filesystem::filesystem_error& e) {
                    DebugLog::addMessage("Failed to delete folder: " + std::string(e.what()));
                }
            }
            m_AssetBrowserState.folderToDelete.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            m_AssetBrowserState.folderToDelete.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

#endif