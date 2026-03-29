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

class BuildSizeAnalyzer {
public:
	void render(const std::vector<Editor::Asset>& assets);
	bool& getShowWindow() { return m_show; }

private:
	void refreshAssets(const std::vector<Editor::Asset>& assets);
	void exportSelected();

	bool m_show = false;
	bool m_dirty = true;
	bool m_wasClosed = true;

	std::vector<BuildAssetEntry> m_entries;
	std::unordered_map<std::string, bool> m_savedSelections; // ensure persist across open/close

	// asset filters
	std::string m_folderFilter = "All";
	std::vector<std::string> m_availableFolders;

	uintmax_t m_totalSelectedSize = 0;

	// export folder dialog state
	char m_exportPath[512] = "";
};