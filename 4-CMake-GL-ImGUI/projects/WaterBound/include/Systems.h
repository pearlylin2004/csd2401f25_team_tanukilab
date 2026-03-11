/* Start Header ************************************************************************/
/*!
\file        Systems.h
\author      Seow Sin Le, s.sinle, 2401084, 35%
\author 	 Hugo Low Ren Hao, low.h, 2402272, 40%
\author 	 Pearly Lin Lee Ying, p.lin, 2401591, 15%
\author 	 Tan Chee Qing, cheeqing.tan, 2401486, 10%
\par         s.sinle@digipen.edu
\par		 low.h@digipen.edu
\par		 p.lin@digipen.edu
\par         cheeqing.tan@digipen.edu
\date        October, 1st, 2025
\brief       System functions to run the logic for the components

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "renderer.h" //RenderSystem needs the full definition of the renderer
#include "input.h" //InputSystem needs access to the InputHandler
#include "GameObjectManager.h"
//#include <stb_image.h>
#include "collision.h"
#include "imgui_internal.h" // for docking in UISystem
//#include <ui.h>
#include "Editor/editorManager.h"
#include "font.h"
#include "messageBus.h"
#include "controllerSystem.h"
#include <LogicContainer.h>
#include "audio.h"
#include <algorithm>
#include <random>
#include <numbers>
#include <vector>

//forward declaration
struct renderer;

//all systems need to have an update function

// Input system - reads user input and updates velocity accordingly
class InputSystem {
public:
	void update(GameObjectManager& manager, const float& deltaTime, MessageBus& messageBus);
};

// Physics system - updates position based on velocity and applies gravity
class PhysicsSystem {
public:
	void update(GameObjectManager& manager, const float& deltaTime, MessageBus& messageBus);

private:
	bool m_stepMode = false;
	bool m_stepReq = false;
};

/*!***********************************************************************
\brief
	render system

*************************************************************************/
class RenderSystem {
	GLuint fbo{};
	GLuint texture{};
	GLuint depthBuffer{};

	int fboWidth = 0;
	int fboHeight = 0;

	void createFBO();
public:
	/*!***********************************************************************
	\brief
		initializes textures for all objects with render component

	\param[in] manager
		array of all objects

	*************************************************************************/
	void init(GameObjectManager& manager, int fboW, int fboH);
	/*!***********************************************************************
	\brief
		resend matrix data to the shaders to recalculated object position and orientation to draw for the new frame

	\param[in] manager
		array of all objects

	*************************************************************************/
	void update(GameObjectManager& manager, float const& deltaTime);
	void renderFBO();
	void resizeFBO(int width, int height);
	GLuint getTexture() const { return texture; }

	//not implemented yet, hopefully tri break can do
	//void renderCollision(Collision::AABB const& box, glm::vec3 const& clr);
	//GLuint getTexture() const { return texture; }
	void batchingSetUp(GameObjectManager& manager, float const& deltaTime);
	void instancePerBatch(std::map<BatchKey, std::vector<renderer::InstanceData>> &batch);
	void clearBatches();

	void fboAspectRatio(int& width, int& height) const;
	~RenderSystem();
	//bool batchRebuild = true;
	//static std::unordered_map<BatchKey, std::vector<renderer::InstanceData>, BatchKeyHash> objectWithTex2;
	//static std::unordered_map<BatchKey, std::vector<renderer::InstanceData>, BatchKeyHash> objectWithTex;
	//static std::unordered_map<BatchKey, std::vector<renderer::InstanceData>, BatchKeyHash> objectWithoutTex;
	static std::map<BatchKey, std::vector<renderer::InstanceData>> objectWithTex;
	static std::map<BatchKey, std::vector<renderer::InstanceData>> objectWithoutTex;
};

class ParticleSystem
{
public:
	//activate particle based on master particle data
	//can add other functions in the future to choose the effects
	void activateParticle(GameObjectManager& manager);
	//currently used for testing purposes only
	//will be used in the future if more effects are added
	void update(GameObjectManager& manager, float const& deltaTime);
	//give each particle a unique ID
	size_t IDGenerator();
	//generate position based on offset
	Vector2D positionGenerator(float offset);
	//generate velocity based on direction
	Vector2D velocityGenerator(Vector2D direction);
	//clear used ID when particle group is finished
	static void clearUsedID(size_t ID);
	//std::vector<size_t> usedID;
};

/*!***********************************************************************
\brief
	Tile Map system

*************************************************************************/
class TileMapSystem
{
public:
	/*!***********************************************************************
	\brief
		update cycle to push the data in tiles to objectWithTex2 to be rendered in render system

	\param[in] manager
		array of all objects

	*************************************************************************/
	void update(GameObjectManager& manager);

	/*!***********************************************************************
	\brief
		update the tiles for the object selected with the texture selected

	\param[in] obj
		object with TileMap component

	*************************************************************************/
	void tileUpdate(GameObject* obj);
	//std::unordered_map<BatchKey, std::vector<renderer::InstanceData>, BatchKeyHash> objectWithTex2{};

	/*!***********************************************************************
	\brief
		to store the filename and filepath of the asset that the mouse has clicked
		and use it to paint the tile map

	*************************************************************************/
	static std::string filename;

	/*!***********************************************************************
	\brief
		mode to erase tiles instead of painting

	*************************************************************************/
	static bool eraseMode;

	static int brushSize;
};

class FontSystem {
public:
	void init(GameObjectManager& manager);
	void update(GameObjectManager& manager, double fps);
	void RenderText(GLuint& s, std::string text, float x, float y, float scale, glm::vec3 color, GameObject* object);

	// sorry very scuffed but can remove aft submission
	static inline bool showFPS = false;
};

/*!***********************************************************************
\brief
	transform system

*************************************************************************/
class TransformSystem {
public:
	/*!***********************************************************************
	\brief
		updating the new position, scale and orientation of the object

	\param[in] GameObjectManager manager
		array of all objects

	\param[in] deltaTime
		tracking of the world time

	*************************************************************************/
	void update(GameObjectManager& manager, const float& deltaTime);

	static void setWorldPosition(Transform* t, float worldX, float worldY, GameObjectManager& manager) {
		t->x = worldX;
		t->y = worldY;

		// Keep localOffset in sync if this object has a parent
		if (!t->parentName.empty()) {
			GameObject* parent = manager.getGameObject(t->parentName);
			if (parent && parent->hasComponent<Transform>()) {
				Transform* pt = parent->getComponent<Transform>();
				float dx = worldX - pt->x;
				float dy = worldY - pt->y;
				float rad = glm::radians(-pt->rotation);
				float cosA = cosf(rad), sinA = sinf(rad);
				t->localOffset.x = dx * cosA - dy * sinA;
				t->localOffset.y = dx * sinA + dy * cosA;
			}
		}
	}

	static void setWorldRotation(Transform* t, float worldRotation, GameObjectManager& manager) {
		// Normalise to [0, 360)
		worldRotation = fmodf(worldRotation, 360.f);
		if (worldRotation < 0.f) worldRotation += 360.f;

		t->rotation = worldRotation;

		if (!t->parentName.empty()) {
			GameObject* parent = manager.getGameObject(t->parentName);
			if (parent && parent->hasComponent<Transform>()) {
				Transform* pt = parent->getComponent<Transform>();
				t->localRotation = worldRotation - pt->rotation;
			}
		}
	}
};

// Collision system - detects and resolves collisions between game objects
class CollisionSystem {
public:
	void update(GameObjectManager& manager, const float& deltaTime);
	bool CheckTileCollision(GameObject* obj, TileMap& map); //for tile map collision
	void HandleTilemapCollisions(const std::unordered_map<int, Collision::Cell>& grid, const std::vector<std::pair<TileMap*, Transform*>>& tilemaps);
};

// only allow editor in debug mode
#ifdef _DEBUG
// UI system - handles UI rendering and interactions using ImGui
class UISystem {
public:
	void init(GLFWwindow* window, RenderSystem* renderer, GameObjectManager& manager);
	~UISystem();
	void update();

	static void toggleUI() { showUI = !showUI; } // temp toggle function
	static bool isShowUI() { return showUI; }


	enum FontType {
		FONT_DEFAULT,
		FONT_ARIAL,
		FONT_TIMES
	};

	static void setFont(FontType type);
	static ImFont* getCurrentFont() { return ImGui::GetIO().FontDefault; }
	static FontType getCurrentFontType() { return m_currentFontType; }

private:
	void dockingSetUp();
	void renderMultipleViewPorts();
	std::unique_ptr<EditorManager> m_UI;
	static bool showUI;
	bool m_firstTime = true;
	RenderSystem* m_renderer = nullptr;

	static ImFont* m_fontDefault;
	static ImFont* m_fontArial;
	static ImFont* m_fontTimes;
	static FontType m_currentFontType;
};
#else
namespace EditorManager {
	inline bool isEditingMode() { return false; }
	inline bool isPaused() { return false; }
}
namespace UISystem {
	inline bool isShowUI() { return false; }
}
#endif

class LogicSystem {
public:
	void update(GameObjectManager& manager, const float& deltaTime);
};

class AudioSystem {
public:
	void init(GameObjectManager & manager);
	void update(GameObjectManager& manager, float deltaTime);
	void cleanup(GameObjectManager& manager);

	// Update 3D positions for all spatial sounds
	void updateSpatialAudio(GameObjectManager& manager);

	static void initializeSceneAudio(GameObjectManager& manager);

private:
	void processAudio(GameObject* obj, AudioChannel* audio, float deltaTime);
};

class VisualLogicSystem {
public:
	void initAll(GameObjectManager& manager);
	void updateAll(GameObjectManager& manager, float dt);

	// init logic component (called once when object is created)
	void init(GameObject* obj, VisualLC* logic);

	// update logic
	void update(GameObject* obj, VisualLC* logic, float dt);

	// broadcast whenever bullet shoot
	static void broadcastBulletShoot();

	void resetLogicState(/*GameObjectManager& manager*/);
private:
	// execute a specific node
	void executeNode(GameObject* obj, VisualLC* logic, int nodeId, float dt);

	// execute different node types
	void executeEventNode(GameObject* obj, VisualLC* logic, LogicNode& node);
	void executeActionNode(GameObject* obj, VisualLC* logic, LogicNode& node, float dt);
	bool executeConditionNode(GameObject* obj, VisualLC* logic, LogicNode& node, float dt);

	// helper to find next connected node
	std::vector<int> findAllNextNode(VisualLC* logic, int fromNodeId, const std::string& outputPin);

	// helper func to find a node by ID
	LogicNode* findNode(VisualLC* logic, int nodeId);

	// helper func for collision
	bool shouldTriggerCollision(LogicNode& node, GameObject* collided);

	static GameObjectManager* m_manager;
};
/*!***********************************************************************
\brief
	Water system for rendering animated water bodies with wave effects
	and applying buoyancy physics to objects in water

*************************************************************************/
class WaterSystem {
public:
	WaterSystem();
	~WaterSystem();

	/*!***********************************************************************
	\brief
		Initialize water shader and uniform locations
	*************************************************************************/
	void init();

	/*!***********************************************************************
	\brief
		Update water movement and apply water physics to objects

	\param[in] manager
		Game object manager containing all objects

	\param[in] deltaTime
		Time elapsed since last frame
	*************************************************************************/
	void update(GameObjectManager& manager, float deltaTime);

	/*!***********************************************************************
	\brief
		Render all water bodies with wave movement

	\param[in] manager
		Game object manager containing water body objects
	*************************************************************************/
	void render(GameObjectManager& manager);

	/*!***********************************************************************
	\brief
		Check if a point is inside a water body

	\param[in] waterObj
		Water body object to check against

	\param[in] point
		2D point to test

	\return
		True if point is in water, false otherwise
	*************************************************************************/
	bool isPointInWater(GameObject* waterObj, const Vector2D& point);

	/*!***********************************************************************
	\brief
		Apply buoyancy and drag forces to objects in water

	\param[in] manager
		Game object manager containing all objects
	*************************************************************************/
	void applyWaterPhysics(GameObjectManager& manager, float deltaTime);

	/*!***********************************************************************
	\brief
		Clean up water meshes and shader resources
	*************************************************************************/
	void cleanup();

private:
	void setupWaterMesh(GameObject* waterObj);
	void renderWaterBody(GameObject* waterObj);

	GLuint m_waterShaderProgram;
	GLint m_timeLocation;
	GLint m_waveAmplitudeLocation;
	GLint m_waveFrequencyLocation;
	GLint m_waveSpeedLocation;
	GLint m_waterAlphaLocation;
	GLint m_waterColorLocation;
	GLint m_useTextureLocation;
	GLint m_viewMatrixLocation;
	GLint m_projMatrixLocation;

	struct WaterMesh {
		GLuint vao;
		GLuint vbo;
		GLuint ebo;
		GLuint ssbo;
		int elementCount;
		int subdivisions;
	};

	std::unordered_map<GameObject*, WaterMesh> m_waterMeshes;
};

class Camera
{
public:
	void update(GameObjectManager& manager);
	void camDefault();
	static void onResize(int w, int h);
	static bool camPresent;
	static int screenWidth;
	static int screenHeight;
};

class WeaponSystem {
public:
	/*!***********************************************************************
	\brief
		Update weapon objects

	\param[in] manager
		Game object manager containing all objects.

	\param[in] deltaTime
		Time elapsed since last frame 
	*************************************************************************/
	void update(GameObjectManager& manager, const float& deltaTime);
};