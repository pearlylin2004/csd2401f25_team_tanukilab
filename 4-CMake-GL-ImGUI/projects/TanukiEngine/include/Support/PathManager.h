/* Start Header ************************************************************************
\file       config.h
\author     Tan Chee Qing, cheeqing.tan, 2401486
\par        cheeqing.tan@digipen.edu
\date       March, 26rd, 2026
\brief      Path manager to provide helper function to get paths to asset folders. 
            The source path is   set by the editor launcher when a project is selected
            and all other paths are derived from it.

Copyright (C) 2026 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
* End Header **************************************************************************/
#pragma once
#include <string>

/*!
 * \class PathManager
 * \brief Singleton that manages file system paths for project assets.
 *
 * Provides a central place to set the project's root source path and
 * retrieve paths to specific asset directories such as Scene, Prefab,
 * Scripts, Audio, Logic, Texture, and Video.
 *
 * All path getters automatically append the relevant subdirectory to
 * the source root. This ensures consistent file locations across the engine.
 */
class PathManager {
public:
    static PathManager& Instance() {
        static PathManager inst;
        return inst;
    }

    void setSourcePath(const std::string& path) { m_sourcePath = path; }
    const std::string& getSourcePath() const { return m_sourcePath; }

    std::string getSourceScenePath()  const { return m_sourcePath + "/assets/Scene/"; }
    std::string getSourcePrefabPath() const { return m_sourcePath + "/assets/Prefab/"; }
    std::string getSourceScriptPath() const { return m_sourcePath + "/assets/scripting/"; }
    std::string getSourceAudioPath()  const { return m_sourcePath + "/assets/Audio/"; }
    std::string getSourceLogicPath() const { return m_sourcePath + "/assets/Logic/"; }
    std::string getSourceTexturePath() const { return m_sourcePath + "/assets/Texture/"; }
    std::string getSourceVideoPath() const { return m_sourcePath + "/assets/video/"; }


private:
    std::string m_sourcePath;
};
