/* Start Header ************************************************************************/
/*!
\file        GameObjectManager.h
\author      Seow Sin Le, s.sinle, 2401084, 20%
\author 	 Hugo Low Ren Hao, low.h, 2402272, 80%
\par         s.sinle@digipen.edu
\par		 low.h@digipen.edu
\date        October, 1st, 2025
\brief       Manages all game objects and initializes them with their corresponding components
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once
#include <fstream>
#include <sstream>
#include <iomanip>
#include <set>

#include "Core/GameObject.h"
#include "Support/layerManager.h"
#include "Support/mathlib.h"

// This class is responsible for creating, storing, and providing access to game objects.
class GameObjectManager {
public:
	/*!
	 * \brief Creates a new game object with the given name.
	 *
	 * Allocates and stores a GameObject internally.
	 *
	 * \param name Name of the new game object.
	 * \return Pointer to the newly created GameObject.
	 */
	GameObject* createGameObject(const std::string& name);

	/*!
	 * \brief Creates a temporary game object representing a prefab (editor-only).
	 *
	 * Does not store the object in the manager's internal map.
	 *
	 * \param name Name of the temporary game object.
	 * \return Unique pointer to the temporary GameObject.
	 */
	static std::unique_ptr<GameObject> createTempGameObject(const std::string& name);

	/*!
	 * \brief Retrieves a stored game object by name.
	 *
	 * Use this when game objects need to interact or reference each other.
	 *
	 * \param name Name of the game object to retrieve.
	 * \return Pointer to the GameObject, or nullptr if not found.
	 */
	GameObject* getGameObject(const std::string& name);

	/*!
	 * \brief Deletes a game object.
	 *
	 * Removes the object from internal storage and frees its memory.
	 *
	 * \param object Pointer to the GameObject to delete.
	 */
	void deleteGameObject(GameObject* object);

	/*!
	 * \brief Clones an existing game object.
	 *
	 * Creates a new object with the same components and copies data.
	 *
	 * \param sourceName Name of the object to clone.
	 * \param newName Name of the new cloned object.
	 * \return Pointer to the cloned GameObject.
	 */
	GameObject* CloneGameObject(const std::string& sourceName, const std::string& newName);

	/*!
	 * \brief Retrieves all currently managed game objects.
	 *
	 * \param gameObjects Vector to populate with pointers to all game objects.
	 */
	void getAllGameObjects(std::vector<GameObject*>& gameObjects);

	/*!
	 * \brief Returns the current number of managed game objects.
	 *
	 * \return Integer count of game objects.
	 */
	int getGameObjectCount();

	/*!
	 * \brief Initializes game objects from a scene or project file.
	 *
	 * Reads saved object data and sets up initial components.
	 */
	void init();

	/*!
	 * \brief Renames an existing game object.
	 *
	 * Updates internal storage to reflect the new name.
	 *
	 * \param obj Pointer to the object to rename.
	 * \param newName New name for the object.
	 * \return True if renaming succeeded; false otherwise.
	 */
	bool renameGameObject(GameObject* obj, const std::string& newName);

	/**
	 * \brief Loads a scene from a JSON file and rebuilds all game objects.
	 * \param filename Source JSON path.
	 *
	 * \details
	 * Behavior:
	 * - Validates version (warns on mismatch, still attempts to load).
	 * - Clears existing scene before creation.
	 * - Accepts booleans and numeric 0/1 for legacy compatibility.
	 * - Defers texture upload by setting \c texHDL=0 and \c texChanged=true when needed.
	 */
	void loadSceneFromJson(const std::string& filename);

	/**
	 * \brief Serializes all current game objects (and components) to a JSON file.
	 * \param filename Destination JSON path.
	 * \param isNew    If \c true , writes only a version header (empty scene shell).
	 *
	 * \details
	 * Layout (version 1):
	 * \code{.json}
	 * {
	 *   "version": 1,
	 *   "objects": [
	 *     {
	 *       "name": "player",
	 *       "components": {
	 *         "Transform": { "pos":[x,y,z], "rotation":r, "scale":[sx,sy,sz] },
	 *         "Render":    { "shape":"square|circle|triangle", "hasTex":true, "texture":"..." , "hasAnimation":0|1 },
	 *         "Input":     {},
	 *         "Physics":   { "inWater":0|1, "buoyancy":0|1 },
	 *         "CollisionInfo": {},
	 *         "Animation": { "loop":0|1, "currentFrameColumn":i, "currentFrameRow":i, "totalColumn":i, "totalRow":i, "frameTime":f }
	 *       }
	 *     }
	 *   ]
	 * }
	 * \endcode
	*/
	void saveSceneToJson(const std::string& filename, bool isNew) const;

	/*!
	 * \brief Clean up and prepare for loads a scene from the specified path by
		calling a helper function.
	 *
	 * Reads object and scene data for simulation or editor use.
	 *
	 * \param path Path to the scene folder or file.
	 */
	void loadScene(const std::string& path);

	/*!
	 * \brief Prepare to saves the current scene to the specified path by calling a 
		helper function.
	 *
	 * \param path Path to save the scene.
	 * \param isNew Optional flag to indicate a new save (default false).
	 */
	void saveScene(const std::string& path, bool isNew = false) const;

	/*!
	* \brief Gets the "on load" script for the scene.
	*
	* \return String containing the script path or contents.
	*/
	std::string getOnLoadScript() const { return m_onLoadScript; }

	/*!
	 * \brief Sets the "on load" script for the scene.
	 *
	 * \param s Script path or contents.
	 */
	void setOnLoadScript(const std::string& s) { m_onLoadScript = s; }

	/*!
	 * \brief Clears all game objects from the current scene.
	 *
	 * Frees memory and resets internal storage.
	 */
	void clearScene(); 

	/*!
	 * \brief Access the LayerManager for layering operations.
	 *
	 * \return Reference to the internal LayerManager.
	 */
	LayerManager& getLayerManager();

	/*!
	 * \brief Assigns a game object to a specific layer.
	 *
	 * \param obj Pointer to the GameObject.
	 * \param layerID ID of the layer to assign to.
	 * \return True if assignment succeeded, false otherwise.
	 */
	bool assignObjectToLayer(GameObject* obj, int layerID);

	/*!
	 * \brief Retrieves all objects in a specific layer.
	 *
	 * \param layerID Layer to query.
	 * \return Vector of pointers to GameObjects in that layer.
	 */
	std::vector<GameObject*> getObjectsInLayer(int layerID);

	/*!
	 * \brief Initializes resources needed when entering a scene.
	 */
	void initializeSceneResources();

	/*!
	 * \brief Initializes resources needed for simulation (runtime).
	 */
	void initializeSimulationResources();

	/*!
	 * \brief Cleans up resources used during simulation.
	 */
	void cleanupSimulationResources();     

	
private:
	//map of all game objects
	std::unordered_map<std::string, std::unique_ptr<GameObject>> m_gameObjects;

	LayerManager m_layerManager;

	std::string m_onLoadScript = "";
};
