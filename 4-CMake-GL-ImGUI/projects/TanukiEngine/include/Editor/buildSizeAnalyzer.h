/* Start Header ************************************************************************/
/*!
\file       buildSizeAnalyzer.h
\author     Tan Chee Qing, cheeqing.tan, 2401486
			- 100%
\par		cheeqing.tan@digipen.edu
\date       March, 29th, 2026
\brief      Class for a build size analyzer in editor for reviewing of project assets and
	        export.

Copyright (C) 2026 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <imgui.h>
#include <vector>
#include <string>
#include <unordered_map>
#include "Editor/editorState.h"

struct BuildAssetEntry {
	std::string name;
	std::string path;
	uintmax_t size = 0;
	bool selected = false;
};

/// @class BuildSizeAnalyzer
/// @brief Tool for analyzing and exporting build asset sizes in the editor.
///
/// BuildSizeAnalyzer provides a UI window to view all assets in the current project,
/// sort them by size, filter by folder, select specific assets, and export selections
/// for further analysis. The class also persists user selections across window open/close.
class BuildSizeAnalyzer {
public:
	/// @brief Renders the Build Size Analyzer window with current asset data.
	/// @param assets List of assets from the editor.
	void render(const std::vector<Editor::Asset>& assets);

	/// @brief Returns a reference to the window visibility flag.
	/// @return Reference to m_show, which controls whether the window is visible.
	bool& getShowWindow() { return m_show; }

private:
	/// @brief Refreshes the internal asset entries from the given list of assets.
	/// @param assets Current editor assets to populate the window.
	void refreshAssets(const std::vector<Editor::Asset>& assets);

	/// @brief Exports currently selected assets to the chosen folder.
	void exportSelected();

	/// @brief Saves currently selected assets to a file to persist across sessions.
	void saveSelections() const;

	/// @brief Loads previously saved asset selections from disk.
	void loadSelections();

	static constexpr const char* k_selectionFile = "editor_build_selection.txt";

	bool m_show = false;
	bool m_dirty = true;
	bool m_wasClosed = true;
	bool m_sortBySize = false;
	bool m_sortAscending = true;
	bool m_showExportResult = false;

	std::vector<BuildAssetEntry> m_entries;
	std::unordered_map<std::string, bool> m_savedSelections; // ensure persist across open/close

	// asset filters
	std::string m_folderFilter = "All";
	std::vector<std::string> m_availableFolders;

	// export folder dialog state
	char m_exportPath[512] = "";

	// for export feedbacks
	float m_exportSuccessTimer = 0.f;
	std::string m_exportResultMsg = "";
	std::vector<std::string> m_overwrittenFiles;
};