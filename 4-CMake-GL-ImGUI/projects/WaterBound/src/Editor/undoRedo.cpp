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

#include "JsonIO.h"
#include "Editor/undoRedo.h"

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
    }
}

CreateObjectCmd::CreateObjectCmd(GameObjectManager& manager, const std::string& objName)
    : m_manager(manager), m_objectName(objName) {}

void CreateObjectCmd::execute() {
    if (!m_wasExecuted) {
        GameObject* obj = m_manager.getGameObject(m_objectName);
        //GameObject* obj = m_createFunc(m_manager);
        if (obj) {
            // serialize current state of obj to a json string
            m_serializedData = JsonIO::serializeGameObj(obj);
        }
        m_wasExecuted = true;
    }
    else {
        if (!m_serializedData.empty()) {
            // deserialize json string to recreate the gameobject
            JsonIO::deserializeGameObj(m_manager, m_serializedData);
        }
    }
}

void CreateObjectCmd::undo() {
    GameObject* obj = m_manager.getGameObject(m_objectName);
    if (obj) {
        m_serializedData = JsonIO::serializeGameObj(obj);

        // reset selection if deleting the obj (to undo creation)
        if (Editor::objSelectionState.selectedObject == obj) {
            Editor::objSelectionState.selectedObject = nullptr;
            Editor::objSelectionState.draggedObject = nullptr;
            Editor::objSelectionState.selectedIndex = -1;
        }

        m_manager.deleteGameObject(obj);
    }
}

DeleteObjCmd::DeleteObjCmd(GameObjectManager& manager, GameObject* obj) : m_manager(manager) {
    if (obj) {
        m_objectName = obj->getObjectName();
        m_layer = obj->getLayer();
        m_serializedData = JsonIO::serializeGameObj(obj);
    }
}

void DeleteObjCmd::execute() {
    GameObject* obj = m_manager.getGameObject(m_objectName);
    if (obj) {
        m_serializedData = JsonIO::serializeGameObj(obj);
        m_manager.deleteGameObject(obj);
    }
}

void DeleteObjCmd::undo() {
    if (!m_serializedData.empty()) {
        GameObject* ori = JsonIO::deserializeGameObj(m_manager, m_serializedData);
        if (ori) {
            m_manager.assignObjectToLayer(ori, m_layer);
        }
    }
}

void UndoRedoManager::executeCmd(std::unique_ptr<CmdInterface> cmd) {
    cmd->execute();
    m_undoStack.push(std::move(cmd));

    while (!m_redoStack.empty()) m_redoStack.pop();

    if (m_undoStack.size() > MAX_UNDO_STEPS) {
        std::stack<std::unique_ptr<CmdInterface>> temp;
        size_t keep = MAX_UNDO_STEPS;

        while (!m_undoStack.empty() && keep > 0) {
            temp.push(std::move(m_undoStack.top()));
            m_undoStack.pop();

            --keep;
        }

        while (!m_undoStack.empty()) m_undoStack.pop();

        while (!temp.empty()) {
            m_undoStack.push(std::move(temp.top()));
            temp.pop();
        }
    }
}

void UndoRedoManager::undo() {
    if (m_undoStack.empty()) return;

    std::unique_ptr<CmdInterface> cmd = std::move(m_undoStack.top());
    m_undoStack.pop();

    cmd->undo();

    m_redoStack.push(std::move(cmd));
}

void UndoRedoManager::redo() {
    if (m_redoStack.empty()) return;

    std::unique_ptr<CmdInterface> cmd = std::move(m_redoStack.top());
    m_redoStack.pop();

    cmd->execute();

    m_undoStack.push(std::move(cmd));
}

void UndoRedoManager::clear() {
    while (!m_undoStack.empty()) m_undoStack.pop();
    while (!m_redoStack.empty()) m_redoStack.pop();
    m_isEditingTransform = false;
}

void UndoRedoManager::beginTransformEdit(GameObjectManager& manager, GameObject* obj) {
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
        auto cmd = std::make_unique<TransformCmd>(manager, m_editingObjName, m_transformBefore, after);
        m_undoStack.push(std::move(cmd));
        while (!m_redoStack.empty()) m_redoStack.pop();

        if (m_undoStack.size() > MAX_UNDO_STEPS) {
            std::stack<std::unique_ptr<CmdInterface>> temp;
            for (size_t i = 0; i < MAX_UNDO_STEPS && !m_undoStack.empty(); ++i) {
                temp.push(std::move(m_undoStack.top()));
                m_undoStack.pop();
            }
            while (!m_undoStack.empty()) m_undoStack.pop();
            while (!temp.empty()) {
                m_undoStack.push(std::move(temp.top()));
                temp.pop();
            }
        }
    }

    m_isEditingTransform = false;
}