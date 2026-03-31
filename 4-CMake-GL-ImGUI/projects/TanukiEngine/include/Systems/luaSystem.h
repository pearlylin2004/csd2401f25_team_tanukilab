/* Start Header ************************************************************************/
/*!
\file       luaSystem.h
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 100% of the file
\par        p.lin@digipen.edu
\date       Ocotber, 23rd, 2025
\brief      Declaration of the LuaSystem class to handle Lua scripting integration. 
			These are the functions that can be used in .lua scripts.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <string>
#include <iostream>
#include "GameObject.h"
#include "input.h"
#include "Component.h"
#include "messageBus.h"
#include "GameObjectManager.h"
#include "GUISystem.h"
#include "Physics.h"
#include "Editor/editorManager.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

/*!
 * \class LuaSystem
 * \brief Handles integration of Lua scripting into the engine for game objects.
 *
 * LuaSystem manages the lifetime of a Lua state, registers engine bindings for
 * game objects, and allows execution of scripts and code strings. It provides
 * getters and setters for physics, rendering, input, audio, FSM, and other
 * systems to Lua scripts, enabling full in-game scripting of behavior and
 * visual logic.
 *
 * Key responsibilities:
 *  - Initialize and clean up the Lua environment.
 *  - Run Lua scripts from files or strings.
 *  - Expose engine data and functions (Transform, Physics, Input, Audio, etc.)
 *    to Lua scripts.
 *  - Update game objects according to their Lua scripts each frame.
 *  - Support editor features such as scene load callbacks and script reloading.
 *
 * Function groups:
 *
 * ---- Initialization / Cleanup ----
 *  - init(manager): Initializes Lua state and registers engine bindings.
 *  - cleanup(): Frees Lua state and resources.
 *  - initializeScripts(manager): Prepares all scripts for objects at scene start.
 *  - callSceneLoad(manager, scriptFile): Calls scene-specific Lua code when a scene is loaded.
 *
 * ---- Script Execution ----
 *  - runString(code): Executes a Lua code string directly.
 *  - runScript(filename): Executes a Lua script from a file.
 *  - getGlobalNumber(name): Retrieves a global numeric variable from Lua.
 *  - loadScriptForObject(filename, tableName): Loads a Lua script for a specific object.
 *  - updateObjectScript(tableName, obj, deltaTime): Updates a single object according to its Lua table.
 *  - update(manager, deltaTime): Updates all registered Lua scripts.
 *
 * ---- Game Object / Engine Bindings ----
 * Exposes C++ engine functionality to Lua scripts, grouped by system:
 *
 * 1. Input
 *    - Lua_getPosition, Lua_setPosition, Lua_IsKeyHeld, Lua_SendInputEvent
 *
 * 2. Render
 *    - Lua_setRenderAlpha, Lua_setTexture, Lua_setColliderSize, Lua_getAnimationFrame
 *
 * 3. Physics (Getters / Setters)
 *    - Lua_getVelX/Y, Lua_setVelX/Y, Lua_getCanMove, Lua_setCanMove, Lua_getJumpForce, etc.
 *    - Supports advanced properties: dynamics, buoyancy, acceleration, gravity timers
 *
 * 4. Collision
 *    - Lua_getCollisionFlag, Lua_setCollisionFlag, Lua_setCollisionTag, Lua_getFriction
 *
 * 5. Transform
 *    - Lua_getRotation/Scale/Offsets, Lua_setRotation/Scale/FlipX
 *
 * 6. Input Bindings
 *    - Lua_isKeyTriggered, Lua_isKeyReleased, Lua_isMouseLeftHeld/Triggered, Gamepad functions
 *
 * 7. FSM / Player State
 *    - Lua_getPlayerState, Lua_setPlayerState, Lua_getFacingRight, Lua_setStateTime
 *
 * 8. Audio
 *    - Lua_playSound, Lua_stopSound, Lua_setSoundVolume, Lua_requestBGM, Lua_setLowPassFilter
 *
 * 9. Visual Script / Logic
 *    - Lua_getVSGlobalFloat, Lua_setVSGlobalFloat, Lua_getNumCurCollisions, Lua_getCurCollision
 *
 * 10. Weapon / Water / Ledge Systems
 *    - Lua_getWeaponCurrentAngle, Lua_setWeaponCurrentAngle, Lua_getWaterBuoyancyForce, Lua_setLedgeClimb, etc.
 *
 * 11. Editor / Debug / Scene
 *    - Lua_isEditingMode, Lua_debugLog, Lua_loadLevel, Lua_getCurrentScene
 *
 * 12. Object Manipulation
 *    - Lua_getAllObjects, Lua_getGameObject, Lua_cloneGameObject, Lua_renameGameObject, Lua_setActive
 *
 * ---- Binding Registration ----
 *  - registerGameBinding(name, fn): Registers a C function callable from Lua.
 *  - registerManagerBindings(manager): Registers all engine bindings (private helper).
 *
 * ---- Accessors ----
 *  - getLuaState(): Returns the underlying lua_State* for advanced use.
 *  - setMessageBus(bus): Sets the message bus for event communication between Lua scripts and engine systems.
 */
class LuaSystem {
public:
    LuaSystem() = default;
    ~LuaSystem() = default;

	void init(GameObjectManager& manager); //initialize Lua state
    void cleanup(); //clean up Lua state
    bool runString(const std::string& code); //execute Lua code from a string
	bool runScript(const std::string& filename); //execute Lua code from a file
	float getGlobalNumber(const std::string& name); //to get the global number variable from Lua
    bool updateObjectScript(const std::string& tableName, GameObject* obj, float deltaTime); //update the script 
    bool loadScriptForObject(const std::string& filename, const std::string& tableName); //load script
	void setMessageBus(MessageBus* bus) { messageBus = bus; } //set message bus
	void update(GameObjectManager& manager, float deltaTime); //update all Lua scripts


	/* -------- On Scene Load ---- */
	void callSceneLoad(GameObjectManager& manager, const std::string& scriptFile);
	void initializeScripts(GameObjectManager& manager);

	/* -------- Register Game Script -------- */
	void registerGameBinding(const std::string& name, lua_CFunction fn);
	lua_State* getLuaState() { return L; }


	/* -------- Input Control -------- */
	/*! \brief Gets the world position of the target object. */
	static int Lua_getPosition(lua_State* L);
	/*! \brief Sets the world position of the target object. */
	static int Lua_setPosition(lua_State* L);
	/*! \brief Returns whether a specified key is currently held down. */
	static int Lua_IsKeyHeld(lua_State* L);
	/*! \brief Sends an input event into the engine for processing. */
	static int Lua_SendInputEvent(lua_State* L);


	/* -------- Render -------- */
	/*! \brief Sets the alpha (transparency) of the target object. */
	static int Lua_setRenderAlpha(lua_State* L);
	/*! \brief Assigns a texture to the target object for rendering. */
	static int Lua_setTexture(lua_State* L);
	/*! \brief Sets the size of the object's collider in the render system. */
	static int Lua_setColliderSize(lua_State* L);
	/*! \brief Gets the current frame of the object's animation. */
	static int Lua_getAnimationFrame(lua_State* L);


	/* -------- Physics Getters -------- */
	/*! \brief Gets the X component of the object's velocity. */
	static int Lua_getVelX(lua_State* L);
	/*! \brief Gets the Y component of the object's velocity. */
	static int Lua_getVelY(lua_State* L);
	/*! \brief Returns true if the object is currently on the ground. */
	static int Lua_isOnGround(lua_State* L);
	/*! \brief Returns whether the object can currently move. */
	static int Lua_getCanMove(lua_State* L);
	/*! \brief Gets the object's movement speed. */
	static int Lua_getMoveSpeed(lua_State* L);
	/*! \brief Gets the object's jump force. */
	static int Lua_getJumpForce(lua_State* L);
	/*! \brief Returns true if the object is currently in water. */
	static int Lua_isInWater(lua_State* L);
	/*! \brief Returns the current gravity affecting the object. */
	static int Lua_getGravity(lua_State* L);
	/*! \brief Returns whether the object can shoot. */
	static int Lua_canShoot(lua_State* L);
	/*! \brief Gets the current shoot timer of the object. */
	static int Lua_getShootTimer(lua_State* L);
	/*! \brief Returns whether the object is alive. */
	static int Lua_isAlive(lua_State* L);
	/*! \brief Gets the fire rate of the object. */
	static int Lua_getFireRate(lua_State* L);
	/*! \brief Gets the remaining life timer of the object. */
	static int Lua_getLifeTimer(lua_State* L);
	/*! \brief Gets the maximum lifetime of the object. */
	static int Lua_getMaxLifetime(lua_State* L);
	/*! \brief Gets the gravity delay timer for the object. */
	static int Lua_getGravityDelayTimer(lua_State* L);
	/*! \brief Gets the gravity delay duration for the object. */
	static int Lua_getGravityDelay(lua_State* L);
	/*! \brief Gets the initial scale of the object. */
	static int Lua_getInitialScale(lua_State* L);
	/*! \brief Gets the minimum scale of the object. */
	static int Lua_getMinScale(lua_State* L);
	/*! \brief Gets the dynamic X velocity of the object. */
	static int Lua_getDynVelX(lua_State* L);
	/*! \brief Gets the dynamic Y velocity of the object. */
	static int Lua_getDynVelY(lua_State* L);
	/*! \brief Gets the dynamic X position of the object. */
	static int Lua_getDynPosX(lua_State* L);
	/*! \brief Gets the dynamic Y position of the object. */
	static int Lua_getDynPosY(lua_State* L);
	/*! \brief Gets the dynamic mass of the object. */
	static int Lua_getDynMass(lua_State* L);
	/*! \brief Gets the coyote time for the object (grace period after leaving ground). */
	static int Lua_getCoyoteTime(lua_State* L);
	/*! \brief Gets the maximum fall speed of the object. */
	static int Lua_getMaxFallSpeed(lua_State* L);
	/*! \brief Returns a single personal boolean flag of the object. */
	static int Lua_getSinlePersonalBool(lua_State* L);
	/*! \brief Returns a single internal flag of the object. */
	static int Lua_getSinleFlag(lua_State* L);
	/*! \brief Returns whether the object has hit the ground. */
	static int Lua_getHitground(lua_State* L);
	/*! \brief Returns the object's single gravity flag. */
	static int Lua_getSinleGravity(lua_State* L);
	/*! \brief Returns whether the object has a water bounce effect. */
	static int Lua_getWaterBounce(lua_State* L);
	/*! \brief Gets the object's X acceleration. */
	static int Lua_getAccelX(lua_State* L);
	/*! \brief Gets the object's Y acceleration. */
	static int Lua_getAccelY(lua_State* L);
	/*! \brief Gets the buoyancy value of the object. */
	static int Lua_getBuoyancy(lua_State* L);


	/* -------- Physics Setters -------- */
	/*! \brief Sets the X component of the object's velocity. */
	static int Lua_setVelX(lua_State* L);
	/*! \brief Sets the Y component of the object's velocity. */
	static int Lua_setVelY(lua_State* L);
	/*! \brief Sets whether the object is on the ground. */
	static int Lua_setOnGround(lua_State* L);
	/*! \brief Sets whether the object can move. */
	static int Lua_setCanMove(lua_State* L);
	/*! \brief Sets the gravity affecting the object. */
	static int Lua_setGravity(lua_State* L);
	/*! \brief Sets the shoot timer of the object. */
	static int Lua_setShootTimer(lua_State* L);
	/*! \brief Sets a physics-related flag on the object. */
	static int Lua_setPhysicsFlag(lua_State* L);
	/*! \brief Applies damping to the object's velocity. */
	static int Lua_applyDamping(lua_State* L);
	/*! \brief Updates the object's position based on physics state. */
	static int Lua_updatePosition(lua_State* L);
	/*! \brief Sets whether the object is alive. */
	static int Lua_setAlive(lua_State* L);
	/*! \brief Sets the life timer of the object. */
	static int Lua_setLifeTimer(lua_State* L);
	/*! \brief Sets the gravity delay timer of the object. */
	static int Lua_setGravityDelayTimer(lua_State* L);
	/*! \brief Sets the dynamic X velocity of the object. */
	static int Lua_setDynVelX(lua_State* L);
	/*! \brief Sets the dynamic Y velocity of the object. */
	static int Lua_setDynVelY(lua_State* L);
	/*! \brief Sets the dynamic position of the object. */
	static int Lua_setDynPos(lua_State* L);
	/*! \brief Updates all forces acting on the object. */
	static int Lua_updateForces(lua_State* L);
	/*! \brief Advances the physics dynamics for the object. */
	static int Lua_advanceDynamics(lua_State* L);
	/*! \brief Sets whether the object is in water. */
	static int Lua_setInWater(lua_State* L);
	/*! \brief Clears all dynamic forces acting on the object. */
	static int Lua_clearDynForces(lua_State* L);
	/*! \brief Sets the original position of the object. */
	static int Lua_setOriginalPos(lua_State* L);
	/*! \brief Returns whether the original state of the object is set. */
	static int Lua_isOriginalStateSet(lua_State* L);
	/*! \brief Enables or disables automatic updates for the object. */
	static int Lua_setAutoUpdate(lua_State* L);
	/*! \brief Sets the X acceleration of the object. */
	static int Lua_setAccelX(lua_State* L);
	/*! \brief Sets the Y acceleration of the object. */
	static int Lua_setAccelY(lua_State* L);
	/*! \brief Sets whether the object has hit the ground. */
	static int Lua_setHitground(lua_State* L);


	/* -------- Collision -------- */
	/*! \brief Gets the collision flag of the object. */
	static int Lua_getCollisionFlag(lua_State* L);
	/*! \brief Sets the collision flag of the object. */
	static int Lua_setCollisionFlag(lua_State* L);
	/*! \brief Sets the collision tag for the object. */
	static int Lua_setCollisionTag(lua_State* L);
	/*! \brief Returns the friction value of the object's collider. */
	static int Lua_getFriction(lua_State* L);


	/* -------- Transform Getters -------- */
	/*! \brief Returns the object's rotation in degrees. */
	static int Lua_getRotation(lua_State* L);
	/*! \brief Sets the object's rotation in degrees. */
	static int Lua_setRotation(lua_State* L);
	/*! \brief Sets the object's scale (X and Y) uniformly or separately. */
	static int Lua_setScale(lua_State* L);
	/*! \brief Returns the object's scale in X axis. */
	static int Lua_getScaleX(lua_State* L);
	/*! \brief Returns the object's scale in Y axis. */
	static int Lua_getScaleY(lua_State* L);
	/*! \brief Returns the local X offset of the object relative to its parent. */
	static int Lua_getLocalOffsetX(lua_State* L);
	/*! \brief Sets the local X offset of the object relative to its parent. */
	static int Lua_setLocalOffsetX(lua_State* L);
	/*! \brief Flips the object horizontally (mirror along X axis). */
	static int Lua_setFlipX(lua_State* L);
	/*! \brief Flips the object horizontally (mirror along X axis). */
	static int Lua_getFlipX(lua_State* L);


	/* -------- Input Bindings -------- */
	/*! \brief Returns true if the specified key is triggered this frame. */
	static int Lua_isKeyTriggered(lua_State* L);
	/*! \brief Returns true if the specified key was released this frame. */
	static int Lua_isKeyReleased(lua_State* L);
	/*! \brief Returns true if the left mouse button is currently held. */
	static int Lua_isMouseLeftHeld(lua_State* L);
	/*! \brief Returns true if the right mouse button is currently held. */
	static int Lua_isMouseRightHeld(lua_State* L);
	/*! \brief Returns true if the left mouse button was triggered this frame. */
	static int Lua_isMouseLeftTriggered(lua_State* L);
	/*! \brief Returns the world X coordinate of the mouse cursor. */
	static int Lua_getMouseWorldX(lua_State* L);
	/*! \brief Returns the world Y coordinate of the mouse cursor. */
	static int Lua_getMouseWorldY(lua_State* L);
	/*! \brief Returns the world X coordinate of the editor mouse. */
	static int Lua_getEditorMouseWorldX(lua_State* L);
	/*! \brief Returns the world Y coordinate of the editor mouse. */
	static int Lua_getEditorMouseWorldY(lua_State* L);
	/*! \brief Returns true if a gamepad is connected. */
	static int Lua_isGamepadConnected(lua_State* L);
	/*! \brief Returns true if a combination of keys is triggered. */
	static int Lua_isComboKeyTriggered(lua_State* L);
	/*! \brief Returns true if a gamepad button is currently held. */
	static int Lua_isGamepadButtonHeld(lua_State* L);
	/*! \brief Returns true if a gamepad button is triggered this frame. */
	static int Lua_isGamepadButtonTriggered(lua_State* L);
	/*! \brief Returns the X rotation (right stick) of the gamepad. */
	static int Lua_getGamepadAxisRX(lua_State* L);
	/*! \brief Returns the Y rotation (right stick) of the gamepad. */
	static int Lua_getGamepadAxisRY(lua_State* L);


	/* -------- FSM -------- */
	/*! \brief Gets the current player state. */
	static int Lua_getPlayerState(lua_State* L);

	/*! \brief Sets the current player state. */
	static int Lua_setPlayerState(lua_State* L);

	/*! \brief Returns true if the current state is fixed and cannot change. */
	static int Lua_isFixState(lua_State* L);

	/*! \brief Sets whether the current state is fixed. */
	static int Lua_setFixState(lua_State* L);

	/*! \brief Returns true if the player is facing right. */
	static int Lua_getFacingRight(lua_State* L);

	/*! \brief Sets whether the player is facing right. */
	static int Lua_setFacingRight(lua_State* L);

	/*! \brief Returns the time elapsed in the current state. */
	static int Lua_getStateTime(lua_State* L);

	/*! \brief Sets the time elapsed in the current state. */
	static int Lua_setStateTime(lua_State* L);


	/* -------- Audio -------- */
	/*! \brief Plays a sound attached to the object. */
	static int Lua_playSound(lua_State* L);

	/*! \brief Stops a sound attached to the object. */
	static int Lua_stopSound(lua_State* L);

	/*! \brief Sets the volume of a sound. */
	static int Lua_setSoundVolume(lua_State* L);

	/*! \brief Returns true if the sound is currently playing. */
	static int Lua_isSoundPlaying(lua_State* L);

	/*! \brief Requests background music to be played. */
	static int Lua_requestBGM(lua_State* L);

	/*! \brief Applies a low-pass filter to the object's audio output. */
	static int Lua_setLowPassFilter(lua_State* L);


	/* -------- Visual Script -------- */
	/*! \brief Gets a global float variable from the visual scripting system. */
	static int Lua_getVSGlobalFloat(lua_State* L);

	/*! \brief Sets a global float variable in the visual scripting system. */
	static int Lua_setVSGlobalFloat(lua_State* L);

	/*! \brief Returns true if the global float exists in the visual scripting system. */
	static int Lua_hasVSGlobalFloat(lua_State* L);

	/*! \brief Returns the number of current collisions detected by visual scripting. */
	static int Lua_getNumCurCollisions(lua_State* L);

	/*! \brief Returns data about the current collision at a specific index. */
	static int Lua_getCurCollision(lua_State* L);


	/* -------- Weapon -------- */
	/*! \brief Returns true if the weapon aims at the mouse cursor. */
	static int Lua_getAimAtMouse(lua_State* L);

	/*! \brief Returns the weapon's base offset in X. */
	static int Lua_getWeaponBaseOffsetX(lua_State* L);

	/*! \brief Sets the weapon's base offset in X. */
	static int Lua_setWeaponBaseOffsetX(lua_State* L);

	/*! \brief Returns the local Y offset of the weapon. */
	static int Lua_getLocalOffsetY(lua_State* L);

	/*! \brief Returns true if the weapon offset has been initialized. */
	static int Lua_isWeaponOffsetInitialised(lua_State* L);

	/*! \brief Sets whether the weapon offset has been initialized. */
	static int Lua_setWeaponOffsetInitialised(lua_State* L);

	/*! \brief Returns the current angle of the weapon. */
	static int Lua_getWeaponCurrentAngle(lua_State* L);

	/*! \brief Sets the current angle of the weapon. */
	static int Lua_setWeaponCurrentAngle(lua_State* L);


	/* -------- WaterBody -------- */
	/*! \brief Returns true if the object has an associated WaterBody component. */
	static int Lua_hasWaterBody(lua_State* L);
	/*! \brief Gets the width of the WaterBody. */
	static int Lua_getWaterWidth(lua_State* L);
	/*! \brief Gets the height of the WaterBody. */
	static int Lua_getWaterHeight(lua_State* L);
	/*! \brief Returns the buoyancy force applied by the WaterBody. */
	static int Lua_getWaterBuoyancyForce(lua_State* L);
	/*! \brief Returns the water drag affecting objects in the WaterBody. */
	static int Lua_getWaterDrag(lua_State* L);
	/*! \brief Gets the rise of the WaterBody in the last frame. */
	static int Lua_getWaterLastFrameRise(lua_State* L);
	/*! \brief Adds height to the WaterBody (e.g., for rising water effects). */
	static int Lua_addWaterHeight(lua_State* L);


	/* -------- Editor State -------- */
	/*! \brief Returns true if the editor is in editing mode. */
	static int Lua_isEditingMode(lua_State* L);
	/*! \brief Returns true if the simulation is currently paused. */
	static int Lua_isPaused(lua_State* L);
	/*! \brief Returns true if the editor UI is open. */
	static int Lua_isEditorOpen(lua_State* L);
	/*! \brief Returns true if the current scene is being hovered by the editor mouse. */
	static int Lua_isSceneHovered(lua_State* L);


	/* -------- Ledge Climb -------- */
	/*! \brief Returns true if the object is currently climbing a ledge. */
	static int Lua_getLedgeClimb(lua_State* L);
	/*! \brief Sets whether the object is climbing a ledge. */
	static int Lua_setLedgeClimb(lua_State* L);
	/*! \brief Gets the target Y position for the ledge climb. */
	static int Lua_getLedgeTargetY(lua_State* L);
	/*! \brief Gets the speed at which the object climbs the ledge. */
	static int Lua_getLedgeClimbSpeed(lua_State* L);
	/*! \brief Returns the jump buffer time for ledge climbing actions. */
	static int Lua_getJumpBufferTime(lua_State* L);


	/* -------- Debug -------- */
	/*! \brief Prints a debug message to the simulation log. */
	static int Lua_debugLog(lua_State* L); // print message in play simulation debug log
	/*! \brief Prints a debug message to the simulation log. */
	static int Lua_debugLogPlay(lua_State* L);


	/* -------- Scene Stuff -------- */
	/*! \brief Prints a debug message to the simulation log. */
	static int Lua_loadLevel(lua_State* L);
	/*! \brief Returns the name of the current scene. */
	static int Lua_getCurrentScene(lua_State* L);


	/* -------- Object Things ---- */
	/*! \brief Returns a list of all objects in the current scene. */
	static int Lua_getAllObjects(lua_State* L);
	/*! \brief Returns a list of all objects in the current scene. */
	static int Lua_getGameObject(lua_State* L);
	/*! \brief Returns the name of the given object. */
	static int Lua_getObjectName(lua_State* L);
	/*! \brief Returns the collision tag of the given object. */
	static int Lua_getCollisionTag(lua_State* L);
	/*! \brief Returns true if the object has a Physics component. */
	static int Lua_hasComponent_Physics(lua_State* L);
	/*! \brief Returns true if the object has a Transform component. */
	static int Lua_hasComponent_Transform(lua_State* L);
	/*! \brief Returns true if the object has a Weapon component. */
	static int Lua_hasWeaponComponent(lua_State* L);
	/*! \brief Returns the name of the object's parent, if it exists. */
	static int Lua_getParentName(lua_State* L);
	/*! \brief Clones the given object and returns the new object. */
	static int Lua_cloneGameObject(lua_State* L);
	/*! \brief Renames the given object to a new name. */
	static int Lua_renameGameObject(lua_State* L);
	/*! \brief Sets whether the object is active in the scene. */
	static int Lua_setActive(lua_State* L);
	/*! \brief Sets whether the object should be excluded from save operations. */
	static int Lua_setExcludeFromSave(lua_State* L);


private:
	lua_State* L = nullptr; //pointer to Lua state
	MessageBus* messageBus = nullptr; //pointer to message bus
	//std::unique_ptr<GameObjectManager> manager; //pointer to game object manager
	GUISystem* guiSystem = nullptr; //pointer to GUI system

	void registerManagerBindings(GameObjectManager& manager);
};
