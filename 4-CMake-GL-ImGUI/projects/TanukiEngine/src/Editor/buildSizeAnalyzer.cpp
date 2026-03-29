/* Start Header ************************************************************************/
/*!
\file       buildSizeAnalyzer.cpp
\author     Tan Chee Qing, cheeqing.tan, 2401486
			- 100%
\par		cheeqing.tan@digipen.edu
\date       March, 29th, 2026
\brief      Function definitionsd for a build size analyzer in editor for reviewing of
			project assets and export.

Copyright (C) 2026 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "Editor/buildSizeAnalyzer.h"
#include "Editor/editorState.h"
#include <windows.h>
#include <shlobj.h>
#include <algorithm>
#include <numeric>

// helper func to format bytes nicely
static std::string formatSize(uintmax_t bytes) {
    if (bytes >= 1024 * 1024)
        return std::to_string(bytes / (1024 * 1024)) + " MB";
    if (bytes >= 1024)
        return std::to_string(bytes / 1024) + " KB";
    return std::to_string(bytes) + " B";
}

// helper to open file explorer
static std::string openFileExplorer() {
	std::string result;

	HRESULT hrInit = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	IFileDialog* pfd = nullptr;
	if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr,
		CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd)))) {

		// set options to pick folders only
		DWORD dwOptions;
		pfd->GetOptions(&dwOptions);
		pfd->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
		pfd->SetTitle(L"Select Export Destination Folder");

		// show file explorer
		if (SUCCEEDED(pfd->Show(nullptr))) {
			IShellItem* psi = nullptr;

			if (SUCCEEDED(pfd->GetResult(&psi))) {
				PWSTR pszPath = nullptr;

				if (SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath))) {
					// convert wide string to narrow string
					char narrow[MAX_PATH];
					WideCharToMultiByte(CP_UTF8, 0, pszPath, -1, narrow, MAX_PATH, nullptr, nullptr);
					result = narrow;
					CoTaskMemFree(pszPath);
				}

				psi->Release();
			}
		}

		pfd->Release();
	}

	if (hrInit == S_OK)
		CoUninitialize();

	return result;
}

void BuildSizeAnalyzer::render(const std::vector<Editor::Asset>& assets) {
	if (!m_show) return;

	if (m_show && m_wasClosed) m_dirty = true;
	m_wasClosed = !m_show;

	if (m_dirty) refreshAssets(assets);

	ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Build Size Analyzer", &m_show)) {
		ImGui::End();
		return;
	}


	/* ---- Select/Deselect All ---- */
	if (ImGui::Button("Select All")) {
		for (BuildAssetEntry& e : m_entries) {
			e.selected = true;
			m_savedSelections[e.path] = true;
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Deselect All")) {
		for (BuildAssetEntry& e : m_entries) {
			e.selected = false;
			m_savedSelections[e.path] = false;
		}
	}


	/* ---- Select/Deselect Selects in this Filter ---- */
	if (m_folderFilter != "All") {
		if (ImGui::Button(("Select All in " + m_folderFilter).c_str())) {
			for (BuildAssetEntry& e : m_entries) {
				std::string f = std::filesystem::path(e.path).parent_path().filename().string();
				if (f == m_folderFilter) { e.selected = true; m_savedSelections[e.path] = true; }
			}
		}

		ImGui::SameLine();

		if (ImGui::Button(("Deselect All in " + m_folderFilter).c_str())) {
			for (BuildAssetEntry& e : m_entries) {
				std::string f = std::filesystem::path(e.path).parent_path().filename().string();
				if (f == m_folderFilter) { e.selected = false; m_savedSelections[e.path] = false; }
			}
		}
	}


	/* ---- Filter ---- */
	ImGui::Text("Filter by folder:");

	ImGui::SameLine();

	ImGui::SetNextItemWidth(200.f);

	if (ImGui::BeginCombo("##folderFilter", m_folderFilter.c_str())) {
		for (const std::string& folder : m_availableFolders) {
			// count assets in the folder
			int count = 0;
			if (folder == "All") {
				count = (int)m_entries.size();
			}
			else {
				for (const BuildAssetEntry& e : m_entries) {
					std::string f = std::filesystem::path(e.path).parent_path().filename().string();
					if (f == folder) ++count;
				}
			}

			std::string displayName = folder + " (" + std::to_string(count) + " assets)";

			bool isSelected = (m_folderFilter == folder);

			if (ImGui::Selectable(displayName.c_str(), isSelected))
				m_folderFilter = folder;

			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndCombo();
	}

	ImGui::Separator();


	/* ---- Asset List ---- */
	ImGui::BeginChild("AssetList", ImVec2(0, -80), true);

	if (ImGui::BeginTable("AssetTable", 3,
		ImGuiTableFlags_Borders |
		ImGuiTableFlags_RowBg |
		ImGuiTableFlags_ScrollY |
		ImGuiTableFlags_SizingStretchProp)) {

		/* -- Table Header -- */
		// headers
		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 30.f);  // checkbox
		ImGui::TableSetupColumn("Asset Name", ImGuiTableColumnFlags_WidthStretch, 1.f);  // asset name
		ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 80.f); // size

		ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
		ImGui::TableSetColumnIndex(0);
		ImGui::TableHeader("");
		ImGui::TableSetColumnIndex(1);
		ImGui::TableHeader("Asset Name");
		ImGui::TableSetColumnIndex(2);

		// clickable size header
		std::string sizeHeader = std::string("Size ") + (m_sortBySize ? (m_sortAscending ? "^" : "v") : "");
		if (ImGui::Selectable(sizeHeader.c_str(), m_sortBySize, ImGuiSelectableFlags_SpanAllColumns)) {
			if (!m_sortBySize) {
				m_sortBySize = true; // first click -> sort ascending
				m_sortAscending = true;
			}
			else if (m_sortAscending) {
				m_sortAscending = false; // second click -> sort descending
			}
			else {
				m_sortBySize = false; // third click -> clear sort
				m_sortAscending = true;
			}
		}

		// sort by size
		std::vector<BuildAssetEntry*> sorted;
		for (BuildAssetEntry& e : m_entries) sorted.push_back(&e);
		if (m_sortBySize) {
			std::sort(sorted.begin(), sorted.end(), [this](BuildAssetEntry* a, BuildAssetEntry* b) {
				return m_sortAscending ? a->size < b->size : a->size > b->size;
				});
		}


		for (BuildAssetEntry* entry : sorted) {
			if (m_folderFilter != "All") {
				std::string entryFolder = std::filesystem::path(entry->path).parent_path().filename().string();
				if (entryFolder != m_folderFilter) continue;
			}

			ImGui::TableNextRow();

			// checkbox column
			ImGui::TableSetColumnIndex(0);
			bool sel = entry->selected;
			if (ImGui::Checkbox(("##chk" + entry->path).c_str(), &sel)) {
				entry->selected = sel;
				m_savedSelections[entry->path] = sel;
			}

			// name column
			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted(entry->name.c_str());

			// size column
			ImGui::TableSetColumnIndex(2);
			ImGui::TextUnformatted(formatSize(entry->size).c_str());
		}

		ImGui::EndTable();
	}

	ImGui::EndChild();


	/* ---- Total Size ---- */
	m_totalSelectedSize = 0;
	uintmax_t filteredSelectedSize = 0;
	size_t filteredSelectedCount = 0;
	size_t totalSelectedCount = 0;

	for (const BuildAssetEntry& e : m_entries) {
		if (e.selected) {
			m_totalSelectedSize += e.size;
			totalSelectedCount++;
		}
		if (m_folderFilter != "All") {
			std::string f = std::filesystem::path(e.path).parent_path().filename().string();
			if (f == m_folderFilter && e.selected) {
				filteredSelectedSize += e.size;
				filteredSelectedCount++;
			}
		}
	}

	ImGui::Separator();
	if (m_folderFilter != "All") {
		ImGui::Text("Current Filter: %s  (%zu selected)", formatSize(filteredSelectedSize).c_str(), filteredSelectedCount);
		ImGui::Text("Total Selected: %s  (%zu selected)", formatSize(m_totalSelectedSize).c_str(), totalSelectedCount);
	}
	else {
		ImGui::Text("Total Selected: %s  (%zu selected)", formatSize(m_totalSelectedSize).c_str(), totalSelectedCount);
	}


	float buttonWidth = 100.f;
	float buttonHeight = ImGui::GetFrameHeight();
	ImVec2 windowSize = ImGui::GetWindowSize();


	/* ---- Export Success Notification ---- */
	if (m_exportSuccessTimer > 0.f) {
		m_exportSuccessTimer -= ImGui::GetIO().DeltaTime;

		ImGui::SetCursorPos(ImVec2(10.f, windowSize.y - buttonHeight - 10.f));
		ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Export successful!");
	}


	/* ---- Export UI ---- */
	ImGui::SetCursorPos(ImVec2(windowSize.x - buttonWidth - 10.f, windowSize.y - buttonHeight - 10.f));

	// can only export if assets selected
	bool hasSelection = (totalSelectedCount > 0);
	if (!hasSelection) ImGui::BeginDisabled();

	if (ImGui::Button(" Export ", ImVec2(buttonWidth, 0))) {
		std::string folder = openFileExplorer();
		if (!folder.empty()) {
			strncpy_s(m_exportPath, folder.c_str(), sizeof(m_exportPath) - 1);
			exportSelected();
		}
	}

	if (!hasSelection) ImGui::EndDisabled();


	ImGui::End();
}

void BuildSizeAnalyzer::refreshAssets(const std::vector<Editor::Asset>& assets) {
	m_entries.clear();
	m_availableFolders.clear();
	m_availableFolders.push_back("All");

	for (const Editor::Asset& asset : assets) {
		std::string folder = std::filesystem::path(asset.path).parent_path().filename().string();
		if (std::find(m_availableFolders.begin(), m_availableFolders.end(), folder) == m_availableFolders.end())
			m_availableFolders.push_back(folder);

		BuildAssetEntry entry;
		entry.name = asset.name;
		entry.path = asset.path;

		std::error_code ec;
		entry.size = std::filesystem::file_size(asset.path, ec);
		if (ec) entry.size = 0;

		auto it = m_savedSelections.find(asset.path);
		entry.selected = (it != m_savedSelections.end()) ? it->second : true; // default select all
		m_entries.push_back(entry);
	}

	m_dirty = false;
}

void BuildSizeAnalyzer::exportSelected() {
	std::filesystem::path dest(m_exportPath);
	if (m_exportPath[0] == '\0') return;

	std::error_code ec;
	std::filesystem::create_directories(dest, ec);

	for (const BuildAssetEntry& entry : m_entries) {
		if (!entry.selected) continue;

		std::filesystem::path src(entry.path);
		std::filesystem::path dst = dest / src.filename();

		std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing, ec);
		if (ec) {
			std::cerr << "[Build Size Analyzer] Failed to export " << entry.name << ": " << ec.message() << std::endl;
		}
	}

	m_exportSuccessTimer = 5.f;
}