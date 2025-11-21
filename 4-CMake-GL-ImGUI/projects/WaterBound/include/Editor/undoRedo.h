#pragma once
#include <string>
#include <memory>
#include <stack>
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

// base class
class CmdInterface {
public:
	virtual ~CmdInterface() = default;

	// these function dont have implementation in this class, any derived class must implement them
	virtual void execute() = 0;
	virtual void undo() = 0;
};

// handle undo/redo for transform changes
class TransformCmd : public CmdInterface {
public:
	// ctor with before/other state
	TransformCmd(GameObjectManager& manager, const std::string& objName, const TransformSnapshot& before, const TransformSnapshot& after);

	// apply AFTER transform snapshot
	void execute() override;

	// apply BEFORE transform snapshot
	void undo() override;

private:
	// helper to add transform data to a game obj
	void applySnapshot(const TransformSnapshot& snap);

	GameObjectManager& m_manager;
	std::string m_objectName;
	TransformSnapshot m_before;
	TransformSnapshot m_after;
};

// handle undo/redo for obj creation
class CreateObjectCmd : public CmdInterface {
public:
	CreateObjectCmd(GameObjectManager& manager, const std::string& objName);

	// create obj (serialize to save it)
	void execute() override;

	// delete the created obj (undo the creation)
	void undo() override;

private:
	GameObjectManager& m_manager;
	std::string m_objectName;
	std::string m_serializedData;
	bool m_wasExecuted = false;
};

// handle undo/redo for obj deletion
class DeleteObjCmd : public CmdInterface {
public:
	DeleteObjCmd(GameObjectManager& manager, GameObject* obj);

	// handle deletion of bj
	void execute() override;

	// recreate obj from serialized data (undo the deletion)
	void undo() override;

private:
	GameObjectManager& m_manager;
	std::string m_objectName;
	std::string m_serializedData;
	int m_layer = 0;
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

	// undo last command
	void undo();

	// redo last command
	void redo();

	// boolean to check if undo/redo is doable
	bool canUndo() const { return !m_undoStack.empty(); }
	bool canRedo() const { return !m_redoStack.empty(); }

	// clear all history (i.e. load scene, stop simulation)
	void clear();

	// start tracking transform changes
	void beginTransformEdit(GameObjectManager& manager, GameObject* obj);

	// ebd tracking and create command if changed detected
	void endTransformEdit(GameObjectManager& manager, GameObject* obj);

	// check if currently editing transform
	bool isEditingTransform() const { return m_isEditingTransform; }

	static constexpr size_t MAX_UNDO_STEPS{ 20 };

private:
	UndoRedoManager() = default;

	std::stack<std::unique_ptr<CmdInterface>> m_undoStack;
	std::stack<std::unique_ptr<CmdInterface>> m_redoStack;

	// for transform
	bool m_isEditingTransform = false;
	std::string m_editingObjName;
	TransformSnapshot m_transformBefore;
};


TransformSnapshot captureTransform(GameObject* obj);