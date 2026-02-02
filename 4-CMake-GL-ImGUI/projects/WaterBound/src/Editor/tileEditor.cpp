/* Start Header ************************************************************************/
/*!
\file       tileEditor.cpp
\author     Seow Sin Le, s.sinle, 2401084 - 70%
			Tan Chee Qing, cheeqing.tan, 2401486 - 30%
\par		s.sinle@digipen.edu
			cheeqing.tan@digipen.edu
\date       November, 26th, 2025
\brief      Definitions of the functions for loading textures onto a tilemap + undo/redo
			tracking for editor

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "Systems.h"
std::unordered_map<BatchKey, std::vector<renderer::InstanceData>, BatchKeyHash> RenderSystem::objectWithTex2{};
bool TileMapSystem::eraseMode = false;
int TileMapSystem::brushSize = 1;

void TileMapSystem::tileUpdate(GameObject* obj)
{
#ifdef _DEBUG
	TileMap* tm = obj->getComponent<TileMap>();
	Transform* transform = obj->getComponent<Transform>();
	Vector2D world = InputHandler::getMousePositionInImGuiViewport(Editor::sceneWindowState.scenePos, Editor::sceneWindowState.sceneSize);
	int centerCol = (int)floor((world.x - transform->x) / tm->tileW);
	int centerRow = (int)floor((world.y - transform->y) / tm->tileH);

	if (SceneWindow::isSceneHovered()) {
		int startX = centerCol - (brushSize - 1) / 2;
		int startY = centerRow - (brushSize - 1) / 2;

		for (int dy = 0; dy < brushSize; ++dy) {
			for (int dx = 0; dx < brushSize; ++dx) {
				int col = startX + dx;
				int row = startY + dy;

				if (col >= -tm->columns && col < tm->columns &&
					row >= -tm->rows && row < tm->rows)
				{
					std::string currentTile = tm->getTile(col, row);

					if (eraseMode) {
						// clear only if tile is not empty
						if (!currentTile.empty()) {
							tm->clearTile(col, row);

							// undo/redo (old -> empty)
							UndoRedoManager::Instance().recordTileChange(col, row, currentTile, "");
						}
					}
					else {
						// reassign only if tile is dff
						if (currentTile != filename) {
							tm->setTile(col, row, filename);

							// undo/redo (old -> new)
							UndoRedoManager::Instance().recordTileChange(col, row, currentTile, filename);
						}
					}

				}
			}
		}
	}
#endif
}

void TileMapSystem::update(GameObjectManager& manager)
{
	std::vector<GameObject*> objects;
	manager.getAllGameObjects(objects);
	RenderSystem::objectWithTex2.clear();

	for (GameObject* obj : objects)
	{
		if (!obj->hasComponent<TileMap>() || !obj->hasComponent<Transform>())
			continue;

#ifdef _DEBUG
		if (InputHandler::isMouseLeftClicked())
		{
			if (!TileMapSystem::filename.empty() && Editor::objSelectionState.selectedObject == obj) {
				UndoRedoManager::Instance().beginTilemapEdit(obj);
			}
		}

		if (InputHandler::isMouseLeftReleased()) {
			if (!TileMapSystem::filename.empty() && Editor::objSelectionState.selectedObject == obj) {
				UndoRedoManager::Instance().endTilemapEdit(manager, obj);
			}
		}
#endif

		if (InputHandler::isMouseLeftHeld())
		{
			//Vector2D world = InputHandler::getMousePositionInImGuiViewport(Editor::sceneWindowState.scenePos, Editor::sceneWindowState.sceneSize);
			//std::cout << world.x << " | " << world.y << std::endl;
			//DebugLog::addMessage("mouse pos: " + std::to_string(world.x) + " | " + std::to_string(world.y) + "\n");
#ifdef _DEBUG
			if (!TileMapSystem::filename.empty() && Editor::objSelectionState.selectedObject == obj) {
				tileUpdate(obj);
			}

#else
			if (!TileMapSystem::filename.empty()) {
				tileUpdate(obj);
			}
#endif
		}

		TileMap* tm = obj->getComponent<TileMap>();
		Transform* transform = obj->getComponent<Transform>(); //assume that all objects has transform

		for (const auto& [tileKey, tileID] : tm->tiles)
		{
			renderer::InstanceData data;
			glm::vec3 pos{ transform->x + tileKey.x * tm->tileW + tm->tileW * 0.5f, transform->y + tileKey.y * tm->tileH + tm->tileH * 0.5f, transform->z };
			glm::mat4 mdl{ 1 };
			mdl = glm::translate(mdl, pos);
			mdl = glm::scale(mdl, glm::vec3(tm->tileW, tm->tileH, 1.f));
			data.model = mdl;
			data.color = glm::vec4(0, 0, 0, 1);
			data.texParams = glm::vec4(0, 0, 1, 1); //no slicing
			TextureData texID = ResourceManager::getInstance().getTexture(tileID);
			RenderSystem::objectWithTex2[BatchKey{ shape::square, texID.id }].push_back(data);
		}
	}

//#ifdef _DEBUG
//	std::cout << "Batches this frame: " << RenderSystem::objectWithTex2.size() << std::endl;
//	for (const auto& [key, instances] : RenderSystem::objectWithTex2) {
//		std::cout << "  Texture ID " << key.texID << ": " << instances.size() << " instances" << std::endl;
//	}
//#endif
}