/* Start Header ************************************************************************/
/*!
\file       undoRedo.h
\author     Tan Chee Qing, cheeqing.tan, 2401486 - 80%
			Hugo Low Ren Hao, low.h, 2402272 - 20%
\par		cheeqing.tan@digipen.edu
			low.h@digipen.edu
\date       November, 10th, 2025
\brief      Class for recording the steps and snapshots for undo and redo. UndoRedoManager
			should be the only public interface needed to track and execute undo and redo
			in the editor.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
//#ifdef _DEBUG
#pragma once
#include <string>
#include <memory>
#include <deque>
#include <functional>
#include "GameObjectManager.h"
#include "Editor/gameDebugLog.h"
#include "Editor/editorState.h"

// transform state of a game obj
struct TransformSnapshot {
	float x{}, y{}, z{};
	float rotation{};
	float scaleX{}, scaleY{}, scaleZ{};
};

// single tile change
struct TileChange {
	int x, y;
	std::string oldTile;
	std::string newTile;
};


// base class
class CmdInterface {
public:
	virtual ~CmdInterface() = default;

	// these function dont have implementation in this class, any derived class must implement them
	virtual void execute() = 0;
	virtual void undo() = 0;

	// to be override in derived class, store obj name (in case of rename)
	virtual void updateObjName(const std::string&, const std::string&) {}
};

// record adding any component to a game obj (work together with componentRegistry in editorState.h)
class AddComponentCmd : public CmdInterface {
public:
	// ctor with obj and component entry (to know which component to add)
	AddComponentCmd(GameObjectManager& manager, GameObject* obj, const Editor::ComponentEntry& entry);
	
	// apply the component addition
	void execute() override;

	// remove the added component (undo the addition)
	void undo() override;

	// if renamed, save the old name and new name (so undo/redo still work)
	void updateObjName(const std::string& oldName, const std::string& newName) override;

private:
	GameObjectManager&		m_manager;
	std::string				m_objectName;
	Editor::ComponentEntry	m_entry;
	std::string				m_serializedData;
};

// record removing any component from a game obj (work together with componentRegistry in editorState.h)
class RemoveComponentCmd : public CmdInterface {
public:
	// ctor with obj and component entry (to know which component to add)
	RemoveComponentCmd(GameObjectManager& manager, GameObject* obj, const Editor::ComponentEntry& entry);

	// apply the component removal
	void execute() override;

	// add the removed component and restores the properties
	void undo() override;

	// if renamed, save the old name and new name (so undo/redo still work)
	void updateObjName(const std::string& oldName, const std::string& newName) override;

private:
	GameObjectManager&		m_manager;
	std::string				m_objectName;
	Editor::ComponentEntry	m_entry;
	std::string				m_serializedData;
};

// store transform changes
class TransformCmd : public CmdInterface {
public:
	// ctor with before/other state
	TransformCmd(GameObjectManager& manager, const std::string& objName, const TransformSnapshot& before, const TransformSnapshot& after,
		const std::unordered_map<std::string, TransformSnapshot>& bystandersBefore,
		const std::unordered_map<std::string, TransformSnapshot>& bystandersAfter);

	// apply AFTER transform snapshot (final position aft transform changed)
	void execute() override;

	// apply BEFORE transform snapshot (put back to initial transform)
	void undo() override;

	// if renamed, save the old name and new name (so undo/redo still work)
	void updateObjName(const std::string& oldName, const std::string& newName);

private:
	// helper to write transform data to a game obj
	void applySnapshot(const TransformSnapshot& snap, bool isUndo);

	GameObjectManager&		m_manager;
	std::string				m_objectName;
	TransformSnapshot		m_before;
	TransformSnapshot		m_after;

	// when other obj is affected cuz of the moved obj (due to collision etc)
	std::unordered_map<std::string, TransformSnapshot> m_bystandersBefore;
	std::unordered_map<std::string, TransformSnapshot> m_bystandersAfter;
};

// store obj creation
class CreateObjectCmd : public CmdInterface {
public:
	CreateObjectCmd(GameObjectManager& manager, const std::string& objName);

	// create obj (serialize to save it)
	void execute() override;

	// delete the created obj (undo the creation)
	void undo() override;

	// if renamed, save the old name and new name (so undo/redo still work)
	void updateObjName(const std::string& oldName, const std::string& newName);

private:
	GameObjectManager&		m_manager;
	std::string				m_objectName;
	std::string				m_serializedData;
	bool					m_wasExecuted = false;
};

// store for obj deletion
class DeleteObjCmd : public CmdInterface {
public:
	DeleteObjCmd(GameObjectManager& manager, GameObject* obj);

	// handle deletion of bj
	void execute() override;

	// recreate obj from serialized data (undo the deletion)
	void undo() override;

	// if renamed, save the old name and new name (so undo/redo still work)
	void updateObjName(const std::string& oldName, const std::string& newName);

private:
	GameObjectManager&		m_manager;
	std::string				m_objectName;
	std::string				m_serializedData;
	int						m_layer = 0;
};

// store tilemap painting changes (not the properties)
class TileMapCmd : public CmdInterface {
public:
	TileMapCmd(GameObjectManager& manager, const std::string& objName, const std::vector<TileChange>& changes);

	// apply new texture
	void execute() override;

	// restore old texture
	void undo() override;

	// if renamed, save the old name and new name (so undo/redo still work)
	void updateObjName(const std::string& oldName, const std::string& newName);

private:
	GameObjectManager&		m_manager;
	std::string				m_objectName;
	std::vector<TileChange>	m_changes;
};

class ReparentCmd : public CmdInterface {
public:
	ReparentCmd(GameObjectManager& manager, const std::string& childName,
		const std::string& oldParent, const Vector2D& oldOffset, float oldLocalRot,
		const std::string& newParent, const Vector2D& newOffset, float newLocalRot);

	void execute() override;
	void undo()    override;

private:
	void apply(const std::string& parent, const Vector2D& offset, float localRot);

	GameObjectManager& m_manager;
	std::string m_childName;
	std::string m_oldParent;  Vector2D m_oldOffset;  float m_oldLocalRot;
	std::string m_newParent;  Vector2D m_newOffset;  float m_newLocalRot;
};

// grid settings & tilemap filename
struct TileMapToolSnapshot {
	std::string filename;
	int brushSize;
	bool eraseMode;
	bool showGrid;
};

class TileMapToolCmd : public CmdInterface {
public:
	TileMapToolCmd(bool& showGrid, const TileMapToolSnapshot& before, const TileMapToolSnapshot& after);

	void execute() override;

	void undo() override;

	void updateObjName(const std::string&, const std::string&) override {}

private:
	bool& m_showGrid;
	TileMapToolSnapshot m_before;
	TileMapToolSnapshot m_after;
	void applySnapshot(const TileMapToolSnapshot& snap);
};

// store any component property changes
class ComponentPropertyCmd : public CmdInterface {
public:
	ComponentPropertyCmd(GameObjectManager& manager, const std::string& objName, const Editor::ComponentEntry& entry, const std::string& before, const std::string& after);

	// apply AFTER state (redo)
	void execute() override;

	// restore BEFORE state (undo)
	void undo() override;

	// if renamed, save the old name and new name (so undo/redo still work)
	void updateObjName(const std::string& oldName, const std::string& newName);
	
private:
	GameObjectManager&		m_manager;
	std::string				m_objectName;
	Editor::ComponentEntry	m_entry;
	std::string				m_before;
	std::string				m_after;
};

//---------------------------
// Command to assign a GameObject to a new layer
// Stores the previous and new layer to allow undo/redo
//---------------------------
class AssignLayerCmd : public CmdInterface {
public:
	AssignLayerCmd(GameObjectManager& manager, GameObject* obj, int layerBefore, int layerAfter);

	void execute() override;

	void undo() override;

	void updateObjName(const std::string& oldName, const std::string& newName) override;

private:
	GameObjectManager&		m_manager;
	std::string				m_objectName;
	int						m_layerBefore;
	int						m_layerAfter;
};

//---------------------------
// Snapshot of a layer's properties
// Used to store layer state for undo/redo
//---------------------------
struct LayerSnapshot {
	bool enabled;
	bool visible;
	bool updatable;

	std::string name;
};

//---------------------------
// Command to modify a layer's properties
// Stores previous and new LayerSnapshot for undo/redo
//---------------------------
class LayerPropertyCmd : public CmdInterface {
public:
	LayerPropertyCmd(GameObjectManager& manager, int layerID, const LayerSnapshot& before, const LayerSnapshot& after);

	void execute() override;

	void undo() override;

	void updateObjName(const std::string&, const std::string&) override {}

private:
	GameObjectManager&		m_manager;
	int						m_layerID;
	LayerSnapshot			m_before;
	LayerSnapshot			m_after;

	void applySnapshot(const LayerSnapshot& snap);
};


//---------------------------
// Snapshot of a visual logic graph
// Stores nodes and connections for undo/redo
//---------------------------
struct VLGraphSnapshot {
	std::vector<LogicNode> nodes;
	std::vector<LogicConnection> connections;
};

//---------------------------
// Command to update a visual logic graph
// Stores previous and new VLGraphSnapshot for undo/redo
//---------------------------
class VLGraphCmd : public CmdInterface {
public:
	VLGraphCmd(const VLGraphSnapshot& before, const VLGraphSnapshot& after);

	void execute() override;

	void undo() override;

	void updateObjName(const std::string&, const std::string&) override;

private:
	void apply(const VLGraphSnapshot& snap);

	VLGraphSnapshot			m_before;
	VLGraphSnapshot			m_after;
};

//---------------------------
// Command to rename a GameObject
// Stores old and new names for undo/redo
//---------------------------
class RenameObjectCmd : public CmdInterface {
public:
	RenameObjectCmd(GameObjectManager& manager, const std::string& oldName, const std::string& newName);
	void execute() override;
	void undo() override;
	void updateObjName(const std::string& oldName, const std::string& newName) override;
private:
	GameObjectManager& m_manager;
	std::string m_oldName;
	std::string m_newName;
};

//---------------------------
// Command to propagate changes from a prefab to its instances
// Stores prefab and instance states before and after propagation
//---------------------------
class PrefabPropagateCmd : public CmdInterface {
public:
	PrefabPropagateCmd(GameObjectManager& manager, const std::string& prefabID,
		std::string prefabBefore, std::string prefabAfter,
		std::unordered_map<std::string, std::string> instancesBefore,
		std::unordered_map<std::string, std::string> instancesAfter);

	void execute() override;
	void undo() override;
	void updateObjName(const std::string&, const std::string&) override {}

private:
	void apply(const std::string& prefabStr, const std::unordered_map<std::string, std::string>& snapshots);

	GameObjectManager&	m_manager;
	std::string			m_prefabID;
	std::string m_prefabBefore;
	std::string m_prefabAfter;
	std::unordered_map<std::string, std::string> m_instancesBefore;
	std::unordered_map<std::string, std::string> m_instancesAfter;
};

//---------------------------
// Command to unpack a prefab from a GameObject
// Removes prefab linkage without deleting the object
//---------------------------
class UnpackPrefabCmd : public CmdInterface {
public:
	UnpackPrefabCmd(GameObjectManager& manager, GameObject* obj);
	void execute() override;
	void undo() override;
	void updateObjName(const std::string& oldName, const std::string& newName) override;
private:
	GameObjectManager& m_manager;
	std::string m_objectName;
	std::string m_prefabID;
};

//---------------------------
// Command to revert a GameObject to match its prefab
// Stores serialized states before and after revert for undo/redo
//---------------------------
class RevertToPrefabCmd : public CmdInterface {
public:
	RevertToPrefabCmd(GameObjectManager& manager, GameObject* obj);
	void execute() override;
	void undo() override;
	void updateObjName(const std::string& oldName, const std::string& newName) override;
private:
	GameObjectManager& m_manager;
	std::string m_objectName;
	std::string m_prefabID;
	std::string m_before; // serialized state before revert
	std::string m_after;  // serialized state after revert
};

//---------------------------
// Command to pack a GameObject into a prefab
// Stores prefab ID for undo/redo; does not delete the prefab
//---------------------------
class PackPrefabCmd : public CmdInterface {
public:
	PackPrefabCmd(GameObjectManager& manager, GameObject* obj);
	void execute() override;
	void undo() override;
	void updateObjName(const std::string& oldName, const std::string& newName) override;
private:
	GameObjectManager& m_manager;
	std::string m_objectName;
	std::string m_prefabID;
};

// big boss for undo/redo operation (singleton)
class UndoRedoManager {
public:
	static UndoRedoManager& Instance() {
		static UndoRedoManager instance;
		return instance;
	}

	// exesute and store a command
	void executeCmd(std::unique_ptr<CmdInterface> cmd);

	// if renamed, save the old name and new name (so undo/redo still work)
	void updateObjName(const std::string& oldName, const std::string& newName);

	// undo last command
	void undo();

	// redo last command
	void redo();

	// boolean to check if undo/redo is doable
	bool canUndo() const { return !m_undoDeque.empty(); }
	bool canRedo() const { return !m_redoDeque.empty(); }

	// save/restore editor history
	void saveEditorHistory();
	void restoreEditorHistory();

	// clear all history (i.e. load scene, stop simulation)
	void clear();

	/* ---- TRANSFORM ---- */
	// start tracking transform changes
	void beginTransformEdit(GameObject* obj);

	// end tracking and create command if changed detected
	void endTransformEdit(GameObjectManager& manager, GameObject* obj);

	// check if currently editing transform
	bool isEditingTransform() const { return m_isEditingTransform; }
	/* ---- END ---- */


	/* ---- TILEMAP ---- */
	// start tracking tilemap changes
	void beginTilemapEdit(GameObject* obj);

	// end tilemap tracking and create command if changes detected
	void endTilemapEdit(GameObjectManager& manager, GameObject* obj);

	// record a single tile change
	void recordTileChange(int x, int y, const std::string& oldTile, const std::string& newTile);

	bool isEditingTilemap() const { return m_isEditingTilemap; }

	void beginTileMapToolEdit(bool showGrid);

	void endTileMapToolEdit(bool showGrid);

	/* ---- END ---- */


	/* ---- COMPONENT PROPERTIES ---- */
	// call before any component property is about to change (captures before-state)
	void beginComponentEdit(GameObject* obj, const Editor::ComponentEntry& entry);

	// call after the change is done (captures after-state, pushes command if changed)
	void endComponentEdit(GameObjectManager& manager, GameObject* obj, const Editor::ComponentEntry& entry);

	bool isEditingComponent() const { return m_isEditingComponent; }
	/* ---- END ---- */

	/* ---- LAYER ---- */
	void beginLayerEdit(int layerID, Layer* layer);
	void endLayerEdit(GameObjectManager& manager, Layer* layer);
	void beginLayerAssign(GameObject* obj);
	void endLayerAssign(GameObjectManager& manager, GameObject* obj, int newLayerID);
	/* ---- END ----*/


	/* ---- VISUAL LOGIC ----*/
	void beginVLEdit();
	void endVLEdit();
	bool isEditingVL() const { return m_isEditingVL; }
	void vlUndo();
	void vlRedo();
	bool canVLUndo() const { return !m_vlUndoDeque.empty(); }
	bool canVLRedo() const { return !m_vlRedoDeque.empty(); }
	void clearVL();
	/* ---- END ----*/

	/* ---- RENAME OBJECT ---- */
	void beginRenameObject(GameObject* obj);
	void endRenameObject(GameObjectManager& manager, GameObject* obj);
	/* ---- END ---- */


	// max steps to undo/redo
	static constexpr size_t MAX_UNDO_STEPS{ 30 };

private:
	UndoRedoManager() = default;

	std::deque<std::unique_ptr<CmdInterface>> m_undoDeque;
	std::deque<std::unique_ptr<CmdInterface>> m_redoDeque;

	// save editor history when playing simulation
	std::deque<std::unique_ptr<CmdInterface>> m_savedEditorUndoDeque;
	std::deque<std::unique_ptr<CmdInterface>> m_savedEditorRedoDeque;

	// for transform
	bool				m_isEditingTransform = false;
	std::string			m_editingObjName;
	TransformSnapshot	m_transformBefore;
	std::unordered_map<std::string, TransformSnapshot> m_bystandersBefore;

	// for tilemap
	bool				m_isEditingTilemap = false;
	std::string			m_editingTilemapObjName;
	std::vector<TileChange> m_tileChanges;
	bool m_isEditingTileMapTool{ false };
	TileMapToolSnapshot m_tileMapToolBefore{};

	// for component properties (all except transform/animation/partial tilemap)
	bool					m_isEditingComponent = false;
	std::string				m_editingComponentObjName;
	Editor::ComponentEntry	m_editingComponentEntry;
	std::string				m_componentBefore;

	// for layer
	bool					m_isEditingLayer{ false };
	int						m_editingLayerID{ -1 };
	LayerSnapshot			m_layerBefore{};
	bool					m_isAssigningLayer{ false };
	std::string				m_assigningObjName{};
	int						m_layerAssignBefore{ -1 };

	// for visual logic
	bool					m_isEditingVL = false;
	VLGraphSnapshot			m_vlBefore;
	std::deque<std::unique_ptr<VLGraphCmd>> m_vlUndoDeque;
	std::deque<std::unique_ptr<VLGraphCmd>> m_vlRedoDeque;

	// for rename object
	bool					m_isRenamingObject = false;
	std::string				m_renameBefore;
};

// take a snapshot of an object's transform state
TransformSnapshot captureTransform(GameObject* obj);
std::string docToString(const rapidjson::Document& doc);
//#endif