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
#ifdef _DEBUG
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
	TransformCmd(GameObjectManager& manager, const std::string& objName, const TransformSnapshot& before, const TransformSnapshot& after);

	// apply AFTER transform snapshot (final position aft transform changed)
	void execute() override;

	// apply BEFORE transform snapshot (put back to initial transform)
	void undo() override;

	// if renamed, save the old name and new name (so undo/redo still work)
	void updateObjName(const std::string& oldName, const std::string& newName);

private:
	// helper to write transform data to a game obj
	void applySnapshot(const TransformSnapshot& snap);

	GameObjectManager&		m_manager;
	std::string				m_objectName;
	TransformSnapshot		m_before;
	TransformSnapshot		m_after;
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

// for layer properties
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

struct LayerSnapshot {
	bool enabled;
	bool visible;
	bool updatable;
};

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


	// max steps to undo/redo
	static constexpr size_t MAX_UNDO_STEPS{ 30 };

private:
	UndoRedoManager() = default;

	std::deque<std::unique_ptr<CmdInterface>> m_undoDeque;
	std::deque<std::unique_ptr<CmdInterface>> m_redoDeque;

	// for transform
	bool				m_isEditingTransform = false;
	std::string			m_editingObjName;
	TransformSnapshot	m_transformBefore;

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
};

// take a snapshot of an object's transform state
TransformSnapshot captureTransform(GameObject* obj);
#endif