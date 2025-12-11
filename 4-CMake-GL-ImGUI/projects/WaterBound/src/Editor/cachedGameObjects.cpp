/* Start Header ************************************************************************/
/*!
\file       cachedGameObjects.h
\author     Tan Chee Qing, cheeqing.tan, 2401486
            - 100% of the file
\par        cheeqing.tan@digipen.edu
\date       December, 11st, 2025
\brief      Definitions of the helper functions that record the cached 
            game objects from manager, and refresh the vector if dirty.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#ifdef _DEBUG
#include "Editor/editorManager.h"

void CachedGameObjects::initialize(GameObjectManager& manager) {
    m_manager = &manager;
	refresh();
}

void CachedGameObjects::refresh() {
    if (!m_manager) {
		DebugLog::addMessage("CachedGameObjects not initialized with GameObjectManager!\n");
        std::cout << "CachedGameObjects not initialized with GameObjectManager!\n";
        return;
    }

    m_cachedObjects.clear();
	m_manager->getAllGameObjects(m_cachedObjects);
}

#endif