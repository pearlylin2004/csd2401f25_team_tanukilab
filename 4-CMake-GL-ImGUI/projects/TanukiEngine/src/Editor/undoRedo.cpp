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
#include "Systems.h"

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

TransformCmd::TransformCmd(GameObjectManager& manager, const std::string& objName, const TransformSnapshot& before, const TransformSnapshot& after, const std::unordered_map<std::string, TransformSnapshot>& bystandersBefore,
    const std::unordered_map<std::string, TransformSnapshot>& bystandersAfter)
    : m_manager(manager), m_objectName(objName), m_before(before), m_after(after), m_bystandersBefore(bystandersBefore), m_bystandersAfter(bystandersAfter) {
}

void TransformCmd::execute() {
    applySnapshot(m_after, false);
}

void TransformCmd::undo() {
    applySnapshot(m_before, true);
}

void TransformCmd::applySnapshot(const TransformSnapshot& snap, bool isUndo) {
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

    auto& bystanders = isUndo ? m_bystandersBefore : m_bystandersAfter;
    for (auto& [name, bSnap] : bystanders) {
        GameObject* other = m_manager.getGameObject(name);
        if (!other) continue;
        Transform* otherT = other->getComponent<Transform>();
        if (otherT) { otherT->x = bSnap.x; otherT->y = bSnap.y; }
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


/* ---------------- Grid Setting -------------- */
TileMapToolCmd::TileMapToolCmd(bool& showGrid, const TileMapToolSnapshot& before, const TileMapToolSnapshot& after)
    : m_showGrid(showGrid), m_before(before), m_after(after) {
}

void TileMapToolCmd::execute() { applySnapshot(m_after); }
void TileMapToolCmd::undo() { applySnapshot(m_before); }

void TileMapToolCmd::applySnapshot(const TileMapToolSnapshot& snap) {
    TileMapSystem::filename = snap.filename;
    TileMapSystem::brushSize = snap.brushSize;
    TileMapSystem::eraseMode = snap.eraseMode;
    m_showGrid = snap.showGrid;
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


/* ---------------- Layer ---------------- */
AssignLayerCmd::AssignLayerCmd(GameObjectManager& manager, GameObject* obj, int layerBefore, int layerAfter)
    : m_manager(manager),
      m_layerBefore(layerBefore),
      m_layerAfter(layerAfter) {
    if (obj) m_objectName = obj->getObjectName();
}

void AssignLayerCmd::execute() {
    GameObject* obj = m_manager.getGameObject(m_objectName);

    if (obj) m_manager.assignObjectToLayer(obj, m_layerAfter);
}

void AssignLayerCmd::undo() {
    GameObject* obj = m_manager.getGameObject(m_objectName);

    if (obj) m_manager.assignObjectToLayer(obj, m_layerBefore);
}

void AssignLayerCmd::updateObjName(const std::string& oldName, const std::string& newName) {
    if (m_objectName == oldName) m_objectName = newName;
}

LayerPropertyCmd::LayerPropertyCmd(GameObjectManager& manager, int layerID, const LayerSnapshot& before, const LayerSnapshot& after)
    : m_manager(manager), m_layerID(layerID), m_before(before), m_after(after) 
{ }

void LayerPropertyCmd::execute() { applySnapshot(m_after); }

void LayerPropertyCmd::undo() { applySnapshot(m_before); }

void LayerPropertyCmd::applySnapshot(const LayerSnapshot& snap) {
    Layer* layer = m_manager.getLayerManager().getLayer(m_layerID);
    if (!layer) return;

    layer->setEnabled(snap.enabled);
    layer->setVisible(snap.visible);
    layer->setUpdatable(snap.updatable);
    if (!snap.name.empty() && snap.name != layer->getLayerName())
        m_manager.getLayerManager().renameLayer(m_layerID, snap.name);
}
/* ---------------- END ---------------- */


/* ---------------- Visual Logic Editor ---------------- */
VLGraphCmd::VLGraphCmd(const VLGraphSnapshot& before, const VLGraphSnapshot& after)
    : m_before(before), m_after(after) 
{ }

void VLGraphCmd::execute() {
    apply(m_after);
}

void VLGraphCmd::undo() {
    apply(m_before);
}

void VLGraphCmd::updateObjName(const std::string&, const std::string&)
{ /* -- no obj in VL editor so this is empty -- */ }

void VLGraphCmd::apply(const VLGraphSnapshot& snap) {
    Editor::visualLogicState.nodes = snap.nodes;
    Editor::visualLogicState.connections = snap.connections;

    // clear selected node if no longer exist
    bool nodeStillExists = std::any_of(snap.nodes.begin(), snap.nodes.end(),
        [](const LogicNode& n) { return n.id == Editor::visualLogicState.selectedNodeId; });

    if (!nodeStillExists)
        Editor::visualLogicState.selectedNodeId = -1;

    // clear selected connection if no longer exist
    bool conStillExists = (Editor::visualLogicState.selectedConIndex >= 0 &&
        Editor::visualLogicState.selectedConIndex < (int)snap.connections.size());

    if (!conStillExists)
        Editor::visualLogicState.selectedConIndex = -1;
}
/* ---------------- END ---------------- */


/* ---------------- RENAME ---------------- */
RenameObjectCmd::RenameObjectCmd(GameObjectManager& manager, const std::string& oldName, const std::string& newName)
    : m_manager(manager), m_oldName(oldName), m_newName(newName) {
}

void RenameObjectCmd::execute() {
    GameObject* obj = m_manager.getGameObject(m_oldName);
    if (obj) m_manager.renameGameObject(obj, m_newName);
}
void RenameObjectCmd::undo() {
    GameObject* obj = m_manager.getGameObject(m_newName);
    if (obj) m_manager.renameGameObject(obj, m_oldName);
}
void RenameObjectCmd::updateObjName(const std::string& oldName, const std::string& newName) {
    if (m_oldName == oldName) m_oldName = newName;
    if (m_newName == oldName) m_newName = newName;
}
/* ---------------- END ---------------- */


/* ---------------- PREFAB PROPOGATE ---------------- */
PrefabPropagateCmd::PrefabPropagateCmd(GameObjectManager& manager, const std::string& prefabID,
    std::string prefabBefore, std::string prefabAfter,
    std::unordered_map<std::string, std::string> instancesBefore,
    std::unordered_map<std::string, std::string> instancesAfter)
    : m_manager(manager), m_prefabID(prefabID),
    m_prefabBefore(std::move(prefabBefore)),
    m_prefabAfter(std::move(prefabAfter)),
    m_instancesBefore(std::move(instancesBefore)),
    m_instancesAfter(std::move(instancesAfter))
{
}

void PrefabPropagateCmd::execute() { apply(m_prefabAfter, m_instancesAfter); }

void PrefabPropagateCmd::undo() { apply(m_prefabBefore, m_instancesBefore); }

void PrefabPropagateCmd::apply(const std::string& prefabStr,
    const std::unordered_map<std::string, std::string>& snapshots) {
    PrefabManager::Instance().loadPrefabFromString(m_prefabID, prefabStr);
    std::string selectedObj;

    // phase 1: delete all first
    for (auto& [name, data] : snapshots) {
        GameObject* obj = m_manager.getGameObject(name);
        if (obj) {
            if (Editor::objSelectionState.selectedObject == obj) {
                selectedObj = name;
                Editor::objSelectionState.selectedObject = nullptr;
                Editor::objSelectionState.draggedObject = nullptr;
                Editor::objSelectionState.selectedIndex = -1;
            }
            m_manager.deleteGameObject(obj);
        }
    }

    // refresh before recreating so nothing iterates stale ptrs
    CachedGameObjects::Instance().refresh();

    // phase 2: recreate all
    for (auto& [name, data] : snapshots) {
        GameObject* restored = JsonIO::deserializeGameObj(m_manager, data);
        if (restored) {
            restored->getObjectName() = name;

            if (name == selectedObj) {
                Editor::objSelectionState.selectedObject = restored;
            }
        }
    }

    CachedGameObjects::Instance().refresh();
    m_manager.initializeSceneResources();
}
std::string docToString(const rapidjson::Document& doc) {
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
    doc.Accept(writer);
    return buf.GetString();
}
/* ---------------- END ---------------- */


/* ---------------- Unpack Prefab ---------------- */
UnpackPrefabCmd::UnpackPrefabCmd(GameObjectManager& manager, GameObject* obj)
    : m_manager(manager) {
    if (obj) {
        m_objectName = obj->getObjectName();
        m_prefabID = obj->getObjectPrefabID();
    }
}

void UnpackPrefabCmd::execute() {
    GameObject* obj = m_manager.getGameObject(m_objectName);
    if (obj) obj->getObjectPrefabID().clear();
}

void UnpackPrefabCmd::undo() {
    GameObject* obj = m_manager.getGameObject(m_objectName);
    if (obj) obj->getObjectPrefabID() = m_prefabID;
}

void UnpackPrefabCmd::updateObjName(const std::string& oldName, const std::string& newName) {
    if (m_objectName == oldName) m_objectName = newName;
}
/* ---------------- END ---------------- */


/* ---------------- Revert To Prefab ---------------- */
RevertToPrefabCmd::RevertToPrefabCmd(GameObjectManager& manager, GameObject* obj)
    : m_manager(manager) {
    if (obj) {
        m_objectName = obj->getObjectName();
        m_prefabID = obj->getObjectPrefabID();
        m_before = JsonIO::serializeGameObj(obj); // capture before state
    }
}

void RevertToPrefabCmd::execute() {
    if (m_after.empty()) {
        // first execute � revert does it, capture after state
        GameObject* obj = m_manager.getGameObject(m_objectName);
        if (!obj) return;

        PrefabManager::Instance().revertToPrefab(obj, m_manager);
        m_after = JsonIO::serializeGameObj(m_manager.getGameObject(m_objectName));
    }
    else {
        // redo � delete and recreate from after snapshot
        GameObject* obj = m_manager.getGameObject(m_objectName);
        if (obj) {
            if (Editor::objSelectionState.selectedObject == obj) {
                Editor::objSelectionState.selectedObject = nullptr;
                Editor::objSelectionState.draggedObject = nullptr;
                Editor::objSelectionState.selectedIndex = -1;
            }
            m_manager.deleteGameObject(obj);
        }

        CachedGameObjects::Instance().refresh();

        GameObject* restored = JsonIO::deserializeGameObj(m_manager, m_after);
        if (restored) {
            restored->getObjectName() = m_objectName;
            Editor::objSelectionState.selectedObject = restored;
        }

        CachedGameObjects::Instance().refresh();
        m_manager.initializeSceneResources();
    }
}

void RevertToPrefabCmd::undo() {
    GameObject* obj = m_manager.getGameObject(m_objectName);
    if (obj) {
        if (Editor::objSelectionState.selectedObject == obj) {
            Editor::objSelectionState.selectedObject = nullptr;
            Editor::objSelectionState.draggedObject = nullptr;
            Editor::objSelectionState.selectedIndex = -1;
        }
        m_manager.deleteGameObject(obj);
    }

    CachedGameObjects::Instance().refresh();
    GameObject* restored = JsonIO::deserializeGameObj(m_manager, m_before);
    if (restored) {
        restored->getObjectName() = m_objectName;
        Editor::objSelectionState.selectedObject = restored;
    }

    CachedGameObjects::Instance().refresh();
    m_manager.initializeSceneResources();
}

void RevertToPrefabCmd::updateObjName(const std::string& oldName, const std::string& newName) {
    if (m_objectName == oldName) m_objectName = newName;
}
/* ---------------- END ---------------- */


/* ---------------- Pack Prefab ---------------- */
PackPrefabCmd::PackPrefabCmd(GameObjectManager& manager, GameObject* obj)
    : m_manager(manager) {
    if (obj) {
        m_objectName = obj->getObjectName();
        m_prefabID = obj->getObjectPrefabID();
    }
}

void PackPrefabCmd::execute() {
    GameObject* obj = m_manager.getGameObject(m_objectName);
    if (obj) obj->getObjectPrefabID() = m_prefabID;
}

void PackPrefabCmd::undo() {
    GameObject* obj = m_manager.getGameObject(m_objectName);
    if (obj) obj->getObjectPrefabID().clear();
}

void PackPrefabCmd::updateObjName(const std::string& oldName, const std::string& newName) {
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

void UndoRedoManager::saveEditorHistory() {
    m_savedEditorUndoDeque = std::move(m_undoDeque);
    m_savedEditorRedoDeque = std::move(m_redoDeque);
    m_undoDeque.clear();
    m_redoDeque.clear();
}
void UndoRedoManager::restoreEditorHistory() {
    m_undoDeque = std::move(m_savedEditorUndoDeque);
    m_redoDeque = std::move(m_savedEditorRedoDeque);
    m_savedEditorUndoDeque.clear();
    m_savedEditorRedoDeque.clear();
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

    m_bystandersBefore.clear();
    if (obj->getComponent<Physics>() && obj->getComponent<CollisionInfo>()) {
        for (GameObject* other : CachedGameObjects::Instance().getCachedObjects()) {
            if (!other || other->getObjectName() == obj->getObjectName()) continue;
            if (!other->getComponent<Physics>() || !other->getComponent<CollisionInfo>()) continue;

            m_bystandersBefore[other->getObjectName()] = captureTransform(other);
        }
    }
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

        std::unordered_map<std::string, TransformSnapshot> bystandersAfter;
        for (auto& [name, snapBefore] : m_bystandersBefore) {
            GameObject* other = manager.getGameObject(name);
            if (!other) continue;
            TransformSnapshot snapAfter = captureTransform(other);
            if (snapBefore.x != snapAfter.x || snapBefore.y != snapAfter.y)
                bystandersAfter[name] = snapAfter;  // only store ones that moved
        }

        auto cmd = std::make_unique<TransformCmd>(manager, m_editingObjName, m_transformBefore, after, m_bystandersBefore, bystandersAfter);

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

//undo redo for parenting
ReparentCmd::ReparentCmd(GameObjectManager& manager, const std::string& childName,
    const std::string& oldParent, const Vector2D& oldOffset, float oldLocalRot,
    const std::string& newParent, const Vector2D& newOffset, float newLocalRot)
    : m_manager(manager), m_childName(childName),
    m_oldParent(oldParent), m_oldOffset(oldOffset), m_oldLocalRot(oldLocalRot),
    m_newParent(newParent), m_newOffset(newOffset), m_newLocalRot(newLocalRot) {
}

void ReparentCmd::execute() { apply(m_newParent, m_newOffset, m_newLocalRot); }

void ReparentCmd::undo() { apply(m_oldParent, m_oldOffset, m_oldLocalRot); }

void ReparentCmd::apply(const std::string& parent, const Vector2D& offset, float localRot) {
    GameObject* child = m_manager.getGameObject(m_childName);
    if (!child) return;
    Transform* t = child->getComponent<Transform>();
    if (!t) return;
    t->parentName = parent;
    t->localOffset = offset;
    t->localRotation = localRot;
}

void UndoRedoManager::beginTileMapToolEdit(bool showGrid) {
    if (m_isEditingTileMapTool) return;

    m_isEditingTileMapTool = true;

    m_tileMapToolBefore = {
        TileMapSystem::filename,
        TileMapSystem::brushSize,
        TileMapSystem::eraseMode,
        showGrid
    };
}

void UndoRedoManager::endTileMapToolEdit(bool showGrid) {
    if (!m_isEditingTileMapTool) return;

    TileMapToolSnapshot after = {
        TileMapSystem::filename,
        TileMapSystem::brushSize,
        TileMapSystem::eraseMode,
        showGrid
    };

    bool changed = (m_tileMapToolBefore.filename != after.filename ||
        m_tileMapToolBefore.brushSize != after.brushSize ||
        m_tileMapToolBefore.eraseMode != after.eraseMode ||
        m_tileMapToolBefore.showGrid != after.showGrid);

    if (changed) {
        auto cmd = std::make_unique<TileMapToolCmd>(Editor::gizmoState.showGrid, m_tileMapToolBefore, after);
        m_undoDeque.push_back(std::move(cmd));
        m_redoDeque.clear();
        if (m_undoDeque.size() > MAX_UNDO_STEPS)
            m_undoDeque.pop_front();
    }

    m_isEditingTileMapTool = false;
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

void UndoRedoManager::beginLayerEdit(int layerID, Layer* layer) {
    if (!layer || m_isEditingLayer) return;

    m_isEditingLayer = true;
    m_editingLayerID = layerID;
    m_layerBefore = { layer->isEnabled(), layer->isVisible(), layer->isUpdatable(), layer->getLayerName() };
}

void UndoRedoManager::endLayerEdit(GameObjectManager& manager, Layer* layer) {
    if (!m_isEditingLayer || !layer) {
        m_isEditingLayer = false;
        return;
    }

    LayerSnapshot after = { layer->isEnabled(), layer->isVisible(), layer->isUpdatable(), layer->getLayerName() };

    bool changed = (m_layerBefore.enabled != after.enabled ||
        m_layerBefore.visible != after.visible ||
        m_layerBefore.updatable != after.updatable ||
        m_layerBefore.name != after.name);

    if (changed) {
        auto cmd = std::make_unique<LayerPropertyCmd>(manager, m_editingLayerID, m_layerBefore, after);

        m_undoDeque.push_back(std::move(cmd));
        m_redoDeque.clear();

        if (m_undoDeque.size() > MAX_UNDO_STEPS)
            m_undoDeque.pop_front();
    }

    m_isEditingLayer = false;
}

void UndoRedoManager::beginLayerAssign(GameObject* obj) {
    if (!obj || m_isAssigningLayer) return;

    m_isAssigningLayer = true;
    m_assigningObjName = obj->getObjectName();
    m_layerAssignBefore = obj->getLayer();
}

void UndoRedoManager::endLayerAssign(GameObjectManager& manager, GameObject* obj, int newLayerID) {
    if (!m_isAssigningLayer || !obj) {
        m_isAssigningLayer = false;
        return;
    }

    if (m_layerAssignBefore != newLayerID) {
        auto cmd = std::make_unique<AssignLayerCmd>(manager, obj, m_layerAssignBefore, newLayerID);
        
        m_undoDeque.push_back(std::move(cmd));
        m_redoDeque.clear();

        if (m_undoDeque.size() > MAX_UNDO_STEPS)
            m_undoDeque.pop_front();
    }

    m_isAssigningLayer = false;
}

void UndoRedoManager::beginVLEdit() {
    if (m_isEditingVL) return;

    m_isEditingVL = true;
    m_vlBefore = { Editor::visualLogicState.nodes, Editor::visualLogicState.connections };
}

void UndoRedoManager::endVLEdit() {
    if (!m_isEditingVL) return;

    m_isEditingVL = false;
    VLGraphSnapshot after = { Editor::visualLogicState.nodes, Editor::visualLogicState.connections };

    // only push when something actually changed
    bool nodesChanged = (m_vlBefore.nodes != after.nodes);
    bool connChanged = (m_vlBefore.connections != after.connections);

    if (nodesChanged || connChanged) {
        auto cmd = std::make_unique<VLGraphCmd>(m_vlBefore, after);
        m_vlUndoDeque.push_back(std::move(cmd));
        m_vlRedoDeque.clear();

        if (m_vlUndoDeque.size() > MAX_UNDO_STEPS)
            m_vlUndoDeque.pop_front();
    }
}

void UndoRedoManager::vlUndo() {
    if (m_vlUndoDeque.empty()) return;

    auto& cmd = m_vlUndoDeque.back();
    cmd->undo();
    m_vlRedoDeque.push_back(std::move(cmd));
    m_vlUndoDeque.pop_back();

    std::cout << "undo queue: " << m_vlUndoDeque.size() << "\n\n";
}

void UndoRedoManager::vlRedo() {
    if (m_vlRedoDeque.empty()) return;
    
    auto& cmd = m_vlRedoDeque.back();
    cmd->execute();
    m_vlUndoDeque.push_back(std::move(cmd));
    m_vlRedoDeque.pop_back();

    std::cout << "undo queue: " << m_vlUndoDeque.size() << "\n\n";

}

void UndoRedoManager::clearVL() {
    m_vlUndoDeque.clear();
    m_vlRedoDeque.clear();
    m_isEditingVL = false;
}

void UndoRedoManager::beginRenameObject(GameObject* obj) {
    if (!obj) return;
    m_isRenamingObject = true;
    m_renameBefore = obj->getObjectName();
}

void UndoRedoManager::endRenameObject(GameObjectManager& manager, GameObject* obj) {
    if (!m_isRenamingObject || !obj) {
        m_isRenamingObject = false;
        return;
    }

    std::string after = obj->getObjectName();
    if (m_renameBefore != after) {
        auto cmd = std::make_unique<RenameObjectCmd>(manager, m_renameBefore, after);
        m_undoDeque.push_back(std::move(cmd));
        m_redoDeque.clear();
        if (m_undoDeque.size() > MAX_UNDO_STEPS)
            m_undoDeque.pop_front();
    }
    m_isRenamingObject = false;
}
/* ---------------- END ---------------- */

#endif