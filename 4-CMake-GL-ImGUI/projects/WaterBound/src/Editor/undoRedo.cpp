/* Start Header ************************************************************************/
/*!
\file       undoRedo.cpp
\author     Tan Chee Qing, cheeqing.tan, 2401486 - 80%
            Hugo Low Ren Hao, low.h, 2402272 - 20%
\par		cheeqing.tan@digipen.edu
            low.h@digipen.edu
\date       November, 10th, 2025
\brief      Definitions of all the functions and ctor needed for undo and redo in editor.
            All components/properties should be able to undo/redo

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#ifdef _DEBUG
#include "JsonIO.h"
#include "Editor/undoRedo.h"
#include "Editor/editorManager.h"

/* ---------------- Add Component ---------------- */
AddComponentCmd::AddComponentCmd(GameObjectManager& manager, GameObject* obj, const Editor::ComponentEntry& entry) : m_manager(manager), m_entry(entry) {
    if (obj) {
        m_objectName = obj->getObjectName();
        m_serializedData = m_entry.serializeComp(obj);
    }
}

void AddComponentCmd::execute() {
    GameObject* obj = m_manager.getGameObject(m_objectName);
    if (!obj /*|| m_serializedData.empty()*/) return;

    m_entry.deserializeComp(obj, m_serializedData);
    // add back component with default values first
    /*if (!m_entry.hasComp(obj)) m_entry.addComp(obj);*/

    // patching?
}

void AddComponentCmd::undo() {
    GameObject* obj = m_manager.getGameObject(m_objectName);
    if (!obj) return;

    if (m_entry.hasComp(obj)) m_entry.removeComp(obj);
}

void AddComponentCmd::updateObjName(const std::string& oldName, const std::string& newName) {
    if (m_objectName == oldName) m_objectName = newName;
}
/* ---------------- END ---------------- */


/* ---------------- Remove Component ---------------- */
RemoveComponentCmd::RemoveComponentCmd(GameObjectManager& manager, GameObject* obj, const Editor::ComponentEntry& entry) : m_manager(manager), m_entry(entry) {
    if (obj) {
        m_objectName = obj->getObjectName();
        m_serializedData = m_entry.serializeComp(obj);

        m_entry.removeComp(obj);
    }
}

void RemoveComponentCmd::execute() {
    GameObject* obj = m_manager.getGameObject(m_objectName);
    if (!obj) return;

    if (m_entry.hasComp(obj)) m_entry.removeComp(obj);
}

void RemoveComponentCmd::undo() {
    GameObject* obj = m_manager.getGameObject(m_objectName);
    if (!obj /*|| m_serializedData.empty()*/) return;

    m_entry.deserializeComp(obj, m_serializedData); // adds if missing, restores data

    //if (!m_entry.hasComp(obj)) m_entry.addComp(obj);

    // patch?
}

void RemoveComponentCmd::updateObjName(const std::string& oldName, const std::string& newName) {
    if (m_objectName == oldName) m_objectName = newName;
}
/* ---------------- END ---------------- */


TransformSnapshot captureTransform(GameObject* obj) {
    TransformSnapshot snap;
    if (!obj) return snap;

    Transform* t = obj->getComponent<Transform>();
    if (t) {
        snap.x = t->x;
        snap.y = t->y;
        snap.z = t->z;

        snap.rotation = t->rotation;

        snap.scaleX = t->scaleX;
        snap.scaleY = t->scaleY;
        snap.scaleZ = t->scaleZ;
    }

    return snap;
}

TransformCmd::TransformCmd(GameObjectManager& manager, const std::string& objName, const TransformSnapshot& before, const TransformSnapshot& after)
    : m_manager(manager), m_objectName(objName), m_before(before), m_after(after) {
}

void TransformCmd::execute() {
    applySnapshot(m_after);
}

void TransformCmd::undo() {
    applySnapshot(m_before);
}

void TransformCmd::applySnapshot(const TransformSnapshot& snap) {
    GameObject* obj = m_manager.getGameObject(m_objectName);
    if (!obj) return;

    Transform* t = obj->getComponent<Transform>();
    if (t) {
        t->x = snap.x;
        t->y = snap.y;
        t->z = snap.z;

        t->rotation = snap.rotation;

        t->scaleX = snap.scaleX;
        t->scaleY = snap.scaleY;
        t->scaleZ = snap.scaleZ;

		// recalculate tilemap collision
        if (TileMap* tm = obj->getComponent<TileMap>())
            tm->colChanged = true;
    }
}

void TransformCmd::updateObjName(const std::string& oldName, const std::string& newName) {
    if (m_objectName == oldName) {
        m_objectName = newName;
    }
}

CreateObjectCmd::CreateObjectCmd(GameObjectManager& manager, const std::string& objName)
    : m_manager(manager), m_objectName(objName) {}

void CreateObjectCmd::execute() {
    // when creating new object
    if (!m_wasExecuted) {
        GameObject* obj = m_manager.getGameObject(m_objectName);
    
        if (obj) {
            // serialize current state of obj to a json string (to store all its component)
            m_serializedData = JsonIO::serializeGameObj(obj);
        }
        m_wasExecuted = true;
    }
    else {
        if (!m_serializedData.empty()) {
            // deserialize json string to recreate the gameobject
            GameObject* obj = JsonIO::deserializeGameObj(m_manager, m_serializedData);

            if (obj) obj->getObjectName() = m_objectName;

            // refresh cached object vector
            CachedGameObjects::Instance().refresh();
        }
    }
}

void CreateObjectCmd::undo() {
    GameObject* obj = m_manager.getGameObject(m_objectName);
    std::cout << "\nfuck" << m_objectName << "\n\n";
    if (obj) {
        m_serializedData = JsonIO::serializeGameObj(obj);

        // reset selection if deleting the obj (to undo creation)
        if (Editor::objSelectionState.selectedObject == obj) {
            Editor::objSelectionState.selectedObject = nullptr;
            Editor::objSelectionState.draggedObject = nullptr;
            Editor::objSelectionState.selectedIndex = -1;
        }

        // delete the obj (to undo create)
        m_manager.deleteGameObject(obj);
        // refresh cached object vector
        CachedGameObjects::Instance().refresh();
    }
}

void CreateObjectCmd::updateObjName(const std::string& oldName, const std::string& newName) {
    if (m_objectName == oldName) {
        m_objectName = newName;

        if (GameObject* obj = m_manager.getGameObject(m_objectName)) {
            m_serializedData = JsonIO::serializeGameObj(obj);
        }
    }
}


DeleteObjCmd::DeleteObjCmd(GameObjectManager& manager, GameObject* obj) : m_manager(manager) {
    if (obj) {
        m_objectName = obj->getObjectName();
        m_layer = obj->getLayer();

        // save the final state before delete
        m_serializedData = JsonIO::serializeGameObj(obj);
    }
}

void DeleteObjCmd::execute() {
    GameObject* obj = m_manager.getGameObject(m_objectName);
    if (obj) {
        // serialize the obj to store its component data
        m_serializedData = JsonIO::serializeGameObj(obj);

        // reset selection if deleting the obj (to undo creation)
        if (Editor::objSelectionState.selectedObject == obj) {
            Editor::objSelectionState.selectedObject = nullptr;
            Editor::objSelectionState.draggedObject = nullptr;
            Editor::objSelectionState.selectedIndex = -1;
        }

        // remove the obj
        m_manager.deleteGameObject(obj);
        // refresh cached object vector
        CachedGameObjects::Instance().refresh();
    }
}

void DeleteObjCmd::undo() {
    if (!m_serializedData.empty()) {
        // create a obj from the stored component data
        GameObject* ori = JsonIO::deserializeGameObj(m_manager, m_serializedData);
        if (ori) {
            ori->getObjectName() = m_objectName;
            m_manager.assignObjectToLayer(ori, m_layer);

            // refresh cached object vector
            CachedGameObjects::Instance().refresh();
        }
    }
}

void DeleteObjCmd::updateObjName(const std::string& oldName, const std::string& newName) {
    if (m_objectName == oldName) {
        m_objectName = newName;
    }
}

TileMapCmd::TileMapCmd(GameObjectManager& manager, const std::string& objName, const std::vector<TileChange>& changes) :
    m_manager(manager),
    m_objectName(objName),
    m_changes(changes)
{ }

void TileMapCmd::execute() {
	GameObject* obj = m_manager.getGameObject(m_objectName);
	if (!obj) return;

	TileMap* tm = obj->getComponent<TileMap>();
	if (!tm) return;

	for (const TileChange& change : m_changes) {
		if (change.newTile.empty()) {
			tm->clearTile(change.x, change.y);
		}
		else {
			tm->setTile(change.x, change.y, change.newTile);
		}
	}
}

void TileMapCmd::undo() {
    GameObject* obj = m_manager.getGameObject(m_objectName);
    if (!obj) return;

    TileMap* tm = obj->getComponent<TileMap>();
    if (!tm) return;

    for (const TileChange& change : m_changes) {
        if (change.oldTile.empty()) {
            tm->clearTile(change.x, change.y);
        }
        else {
            tm->setTile(change.x, change.y, change.oldTile);
        }
    }
}

void TileMapCmd::updateObjName(const std::string& oldName, const std::string& newName) {
	if (m_objectName == oldName)
		m_objectName = newName;
}
/* ---------------- END ---------------- */


/* ---------------- Component Property ---------------- */
ComponentPropertyCmd::ComponentPropertyCmd(GameObjectManager& manager, const std::string& objName, const Editor::ComponentEntry& entry, const std::string& before, const std::string& after)
    :   m_manager(manager),
        m_objectName(objName),
        m_entry(entry),
        m_before(before),
        m_after(after)
{}

void ComponentPropertyCmd::execute() {
    GameObject* obj = m_manager.getGameObject(m_objectName);

    if (obj) m_entry.deserializeComp(obj, m_after);
}

void ComponentPropertyCmd::undo() {
    GameObject* obj = m_manager.getGameObject(m_objectName);

    if (obj) m_entry.deserializeComp(obj, m_before);
}

void ComponentPropertyCmd::updateObjName(const std::string& oldName, const std::string& newName) {
    if (m_objectName == oldName) m_objectName = newName;
}
/* ---------------- END ---------------- */


/* ---------------- UNDO/REDO ---------------- */
void UndoRedoManager::executeCmd(std::unique_ptr<CmdInterface> cmd) {
    // execute the command (transform/create/delete)
    cmd->execute();

    // push to undo deque
    m_undoDeque.push_back(std::move(cmd));

    // clear redo history
    m_redoDeque.clear();

    // trim oldest one if too large (more than max steps)
    if (m_undoDeque.size() > MAX_UNDO_STEPS) {
        m_undoDeque.pop_front();
    }
}

void UndoRedoManager::updateObjName(const std::string& oldName, const std::string& newName) {
    for (std::unique_ptr<CmdInterface>& cmd : m_undoDeque) {
        cmd->updateObjName(oldName, newName);
    }

    for (std::unique_ptr<CmdInterface>& cmd : m_redoDeque) {
        cmd->updateObjName(oldName, newName);
    }
}

void UndoRedoManager::undo() {
    if (m_undoDeque.empty()) return;

    // take the last command from undo deque
    std::unique_ptr<CmdInterface> cmd = std::move(m_undoDeque.back());
    m_undoDeque.pop_back();

    // undo it
    cmd->undo();

    // push the undo step to redo deque
    m_redoDeque.push_back(std::move(cmd));
}

void UndoRedoManager::redo() {
    if (m_redoDeque.empty()) return;

    std::unique_ptr<CmdInterface> cmd = std::move(m_redoDeque.back());
    m_redoDeque.pop_back();

    cmd->execute();

    m_undoDeque.push_back(std::move(cmd));
}

void UndoRedoManager::clear() {
    m_undoDeque.clear();
    m_redoDeque.clear();
    m_isEditingTransform = false;
    m_isEditingComponent = false;
}

void UndoRedoManager::beginTransformEdit(GameObject* obj) {
    //(void)manager;
    if (!obj || m_isEditingTransform) return;

    m_isEditingTransform = true;
    m_editingObjName = obj->getObjectName();
    m_transformBefore = captureTransform(obj);
}

void UndoRedoManager::endTransformEdit(GameObjectManager& manager, GameObject* obj) {
    if (!m_isEditingTransform || !obj) {
        m_isEditingTransform = false;
        return;
    }

    if (obj->getObjectName() != m_editingObjName) {
        m_isEditingTransform = false;
        return;
    }

    TransformSnapshot after = captureTransform(obj);

    // only create when something changed
    bool changed = (m_transformBefore.x != after.x || m_transformBefore.y != after.y ||
        m_transformBefore.z != after.z || m_transformBefore.rotation != after.rotation ||
        m_transformBefore.scaleX != after.scaleX || m_transformBefore.scaleY != after.scaleY ||
        m_transformBefore.scaleZ != after.scaleZ);

    if (changed) {
        if (TileMap* tm = obj->getComponent<TileMap>()) {
            tm->colChanged = true;
        }

        auto cmd = std::make_unique<TransformCmd>(manager, m_editingObjName, m_transformBefore, after);

        // add to undo deque
        m_undoDeque.push_back(std::move(cmd));

        // clear redo
        m_redoDeque.clear();

        // trim oldest step if more than 20 steps saved
        if (m_undoDeque.size() > MAX_UNDO_STEPS) {
            m_undoDeque.pop_front();
        }
    }

    m_isEditingTransform = false;
}

void UndoRedoManager::beginTilemapEdit(GameObject* obj) {
	if (!obj || m_isEditingTilemap) return;

	m_isEditingTilemap = true;
	m_editingTilemapObjName = obj->getObjectName();
    m_tileChanges.clear();
}

void UndoRedoManager::endTilemapEdit(GameObjectManager& manager, GameObject* obj) {
    if (!obj || !m_isEditingTilemap) {
        m_isEditingTilemap = false;
		m_tileChanges.clear();
        return;
    }

    if (obj->getObjectName() != m_editingTilemapObjName) {
        m_isEditingTilemap = false;
        m_tileChanges.clear();
        return;
    }

    if (!m_tileChanges.empty()) {
        auto cmd = std::make_unique<TileMapCmd>(manager, m_editingTilemapObjName, m_tileChanges);
        
        // add to undo deque
        m_undoDeque.push_back(std::move(cmd));

        // clear redo
        m_redoDeque.clear();

        // trim oldest step if more than MAX steps saved
        if (m_undoDeque.size() > MAX_UNDO_STEPS)
            m_undoDeque.pop_front();
    }

	m_isEditingTilemap = false;
    m_tileChanges.clear();
}

void UndoRedoManager::recordTileChange(int x, int y, const std::string& oldTile, const std::string& newTile) {
	if (!m_isEditingTilemap) return;

    // check if alr record this tile in this edit session
	for (TileChange& change : m_tileChanges) {
		if (change.x == x && change.y == y) {
			change.newTile = newTile;
			return;
		}
	}

	m_tileChanges.push_back({ x, y, oldTile, newTile });
}

void UndoRedoManager::beginComponentEdit(GameObject* obj, const Editor::ComponentEntry& entry) {
    if (!obj || m_isEditingComponent) return;

    m_isEditingComponent = true;
    m_editingComponentObjName = obj->getObjectName();
    m_editingComponentEntry = entry;
    m_componentBefore = entry.serializeComp(obj);
}

void UndoRedoManager::endComponentEdit(GameObjectManager& manager, GameObject* obj, const Editor::ComponentEntry& entry) {
    if (!m_isEditingComponent || !obj) {
        m_isEditingComponent = false;
        return;
    }

    if (obj->getObjectName() != m_editingComponentObjName) {
        m_isEditingComponent = false;
        return;
    }

    std::string after = entry.serializeComp(obj);

    // only push if something actually changed
    if (after != m_componentBefore) {
        auto cmd = std::make_unique<ComponentPropertyCmd>(
            manager, m_editingComponentObjName, entry, m_componentBefore, after);

        m_undoDeque.push_back(std::move(cmd));
        m_redoDeque.clear();

        if (m_undoDeque.size() > MAX_UNDO_STEPS)
            m_undoDeque.pop_front();
    }

    m_isEditingComponent = false;
}
/* ---------------- END ---------------- */

#endif