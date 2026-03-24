
#include "GameApp.h"
#include "GameObject.h"
#include "Component.h"
#include "GameObjectManager.h"
//#include "input.h"
//#include "messageBus.h"
//#include "message.h"
#include "Physics.h"
#include "luaSystem.h"
// #include "editorManager.h"
// #include "sceneWindow.h"
//#include <iostream>
#include <vector>

void GameApp::onSceneLoad(GameObjectManager& manager) {
    if (m_luaSystem) m_luaSystem->callSceneLoad(manager);

//    m_manager = &manager;
//
//    // build bullet pool
//    PlayerScript::createBulletPool(manager, "bullet", 20);
// 
//    m_waterSystem.init();
// 
//#ifdef _DEBUG
//    std::vector<GameObject*> objs;
//    manager.getAllGameObjects(objs);
//    std::cout << "\n[GameApp] ====== OBJECT FSM ASSIGNMENT ======\n";
//    for (auto* go : objs) {
//        if (!go) continue;
//        CollisionInfo* col = go->getComponent<CollisionInfo>();
//        // std::cout << "  " << go->getObjectName()
//        //           << " | tag: "     << (col ? col->collisionTag : "(none)")
//        //           << " | SM: "      << (go->hasComponent<StateMachine>()  ? "Y" : "N")
//        //           << " | Input: "   << (go->hasComponent<Input>()         ? "Y" : "N")
//        //           << " | Physics: " << (go->hasComponent<Physics>()       ? "Y" : "N")
//        //           << "\n";
//    }
//    std::cout << "[GameApp] =========================================\n\n";
//#endif
}
// 
//void GameApp::onSceneUnload(GameObjectManager& /*manager*/) {
//void GameApp::onSceneUnload(GameObjectManager& /*manager*/) {
//    // LogicContainer clears naturally — m_machines keyed on raw ptrs that become invalid
//}
// 
//// ============================================================================
//// LogicSystem hooks
//// ============================================================================
// 
void GameApp::update(GameObject& obj, float dt) {
    if (obj.hasComponent<Input>()) return;
    if (!obj.hasComponent<LuaScript>()) return;
    LuaScript* script = obj.getComponent<LuaScript>();
    for (auto& entry : script->scripts)
        if (!entry.scriptName.empty() && !entry.filename.empty())
            m_luaSystem->updateObjectScript(entry.scriptName, &obj, dt);
//    m_container.update(obj, dt);
}

void GameApp::updateEnvironment(GameObject& obj, float dt) {
    if (obj.hasComponent<Input>()) return;
    if (!obj.hasComponent<LuaScript>()) return;
    LuaScript* script = obj.getComponent<LuaScript>();
    for (auto& entry : script->scripts)
        if (!entry.scriptName.empty() && !entry.filename.empty())
            m_luaSystem->updateObjectScript(entry.scriptName, &obj, dt);
//    CollisionInfo* col = obj.getComponent<CollisionInfo>();
//    if (!col) return;
// 
//    const std::string& tag = col->collisionTag;
// 
//    if (tag == "fire")
//        m_container.updateFire(obj, dt);
//    else if (tag == "water") {
//        static bool once = true;
//        if (once) {
//            std::cout << "[GameApp] updateRefill() called for: " << obj.getObjectName() << "\n";
//            once = false;
//        }
//        m_container.updateRefill(obj, dt);
//    }
}
// 
void GameApp::updateAll(GameObjectManager& manager, float dt) {
    if (m_luaSystem) m_luaSystem->update(manager, dt);
//    m_waterSystem.update(manager, dt);
//    m_buoyancy.update(manager, dt);
//    m_weaponSystem.update(manager, dt);
}

bool GameApp::onCollision(GameObject* a, GameObject* b, CollisionInfo& info, GameObjectManager& manager) {
    (void)info;
    (void)manager;
 
    CollisionInfo* ca = a->getComponent<CollisionInfo>();
    CollisionInfo* cb = b->getComponent<CollisionInfo>();
    if (!ca || !cb) return false;
 
    const std::string& tagA = ca->collisionTag;
    const std::string& tagB = cb->collisionTag;

    // floatingObject entering water body
    if ((tagA == "floatingObject" && b->hasComponent<WaterBody>()) ||
        (tagB == "floatingObject" && a->hasComponent<WaterBody>())) {
        GameObject* floater = (tagA == "floatingObject") ? a : b;
        Physics* p = floater->getComponent<Physics>();
        if (p) p->inWater = true;
        return false; // don't skip engine response
    }
 
    // -- bullet + bullet → water particle merge --
    if (tagA == "bullet" && tagB == "bullet") {
        //if (!EditorManager::isEditingMode()) {
            return Collision::HandleWaterParticleMerge(a, b); // true = skip engine
        //}
        return false;
    }
 
    // -- bullet + water → deactivate bullet, raise water level --
    bool bulletIsA = (tagA == "bullet" && tagB == "water");
    bool bulletIsB = (tagB == "bullet" && tagA == "water");
    if (bulletIsA || bulletIsB) {
        GameObject* bullet    = bulletIsA ? a : b;
        GameObject* waterObj  = bulletIsA ? b : a;
        deactivateBullet(bullet);
        WaterBody* water = waterObj->getComponent<WaterBody>();
        if (water) water->pendingHeightIncrease += water->heightIncrease;
        return true; // skip engine response
    }
 
    return false; // engine handle everything else
}

bool GameApp::onCollisionResponse(GameObject*    obj, CollisionInfo* col, CollisionInfo& info, bool isStatic, CollisionInfo* otherCol) {
    if (!col || !otherCol) return false;
 
    const std::string& tag      = col->collisionTag;
    const std::string& otherTag = otherCol->collisionTag;
    Physics*   p = obj->getComponent<Physics>();
    Transform* t = obj->getComponent<Transform>();
    if (!p || !t) return false;
 
    // -- bullet: special ground/wall bounce (12.0f redirect) --
    if (tag == "bullet") {
        if (info.normal.x != 0) {
            // Bullet hits a wall — skip push-out (let it pass or stop naturally)
            return true;
        }
        if (info.normal.y != 0) {
            // Bullet hits floor/ceiling — redirect horizontally
            float direction = (p->dynamics.velocity.x >= 0.f) ? 1.f : -1.f;
            p->dynamics.velocity.x = 12.0f * direction;
            p->dynamics.velocity.y = 0.0f;
            p->onGround            = true;
            return true;
        }
    }
 
    // -- floatingObject: impact dip impulse when player lands on it --
    if (otherTag == "floatingObject" && info.normal.y > 0 && p->velY < -1.0f) {
        // obj landed on top of a floating platform — push it down
        Physics* floatP = obj->getComponent<Physics>(); // reuse p (obj is the lander)
        // find the floating object's physics
        // note: this is called for 'obj' which is the lander, not the floater
        // the engine will call this for both objects in the pair
        // so we check if otherCol is floatingObject
        (void)floatP;
        (void)isStatic;
        return false;
 
    if (tag == "floatingObject" && otherTag != "bullet") {
        if (info.normal.y < 0 && !isStatic) {
            // Something landed on us from above
            float impactFraction = 0.7f;
            float approxImpactVel = -std::abs(info.penetration) * 60.0f; // estimate
            float massRatio = 1.0f; // can't easily get other mass here without refactor
            p->velY += approxImpactVel * impactFraction * massRatio;
            p->velY = std::max(p->velY, -15.0f);
            p->dynamics.velocity.y = p->velY;
        }
        return false; // engine still zeroes the lander's velocity
    }
 
    return false; // engine handles everything else normally
    }
}

Vector2D GameApp::getAudioListenerPos(GameObjectManager& manager) {
    GameObject* player = manager.getGameObject("player");
    if (player && player->hasComponent<Transform>()) {
        Transform* t = player->getComponent<Transform>();
        return { t->x, t->y };
    }
    return { 0.f, 0.f };
}

//void GameApp::registerLuaBindings(LuaSystem& lua) {
//    lua.registerGameBinding("doJump", [](lua_State* L) -> int {
//        GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
//        if (obj) PlayerScript::jump(obj);
//        return 0;
//        });
//
//    lua.registerGameBinding("doShoot", [](lua_State* L) -> int {
//        GameObject* bullet = static_cast<GameObject*>(lua_touserdata(L, 1));
//        GameObject* origin = static_cast<GameObject*>(lua_touserdata(L, 2));
//
//        if (!bullet || !origin) return 0;
//        Transform* t = origin->getComponent<Transform>();
//        if (!t) return 0;
//
//        float tx = static_cast<float>(lua_tonumber(L, 3));
//        float ty = static_cast<float>(lua_tonumber(L, 4));
//
//        PlayerScript::shoot(bullet, t, { tx, ty });
//        return 0;
//        });
//
//    lua.registerGameBinding("deactivateBullet", [](lua_State* L) -> int {
//        GameObject* obj = static_cast<GameObject*>(lua_touserdata(L, 1));
//        if (obj) PlayerScript::deactivateBullet(obj);
//        return 0;
//        });
//
//    if (m_manager) {
//        lua_State* L = lua.getLuaState();
//        lua_pushlightuserdata(L, m_manager);
//        lua_pushcclosure(L, [](lua_State* L) -> int {
//            GameObjectManager* mgr = static_cast<GameObjectManager*>(
//                lua_touserdata(L, lua_upvalueindex(1)));
//            if (!mgr) { lua_pushnil(L); return 1; }
//            GameObject* bullet = PlayerScript::findAvailableBullet(*mgr);
//            if (bullet) lua_pushlightuserdata(L, bullet);
//            else        lua_pushnil(L);
//            return 1;
//            }, 1);
//        lua_setglobal(L, "findAvailableBullet");
//    }
//}

void GameApp::onPlayerInput(GameObject& obj, GameObjectManager& manager, MessageBus&        messageBus, float dt) {
    if (!obj.hasComponent<LuaScript>()) return;
    LuaScript* script = obj.getComponent<LuaScript>();
    for (auto& entry : script->scripts)
    if (!entry.scriptName.empty() && !entry.filename.empty())
    m_luaSystem->updateObjectScript(entry.scriptName, &obj, dt);
//    Physics* physics = obj.getComponent<Physics>();
//    Transform* transform = obj.getComponent<Transform>();
//    if (!physics || !transform) return;
//
//    /* ---- SIN LE PHYSICS for branch ---- */
//    if (physics->sinlePersonalBool) {
//        physics->physicsFlag = false;
//        if (physics->sinleFlag) {
//            physics->acceleration.y = physics->inWater ? physics->waterBounce : 0.f;
//            physics->velX += physics->acceleration.x * dt;
//            physics->velY += (physics->acceleration.y + physics->sinleGravity) * dt;
//            if (physics->hitWater)  { /* onEnterWater handled by BuoyancySystem */ }
//            if (physics->hitground) physics->velY = 0.f;
//            transform->x += physics->velX * dt;
//            transform->y += physics->velY * dt;
//        }
//        return;
//    }
//    /* ---- END ---- */
//
//
//    /* ---- JUMP ----  */
//    if (transform->y <= physics->floorY && !obj.checkAutoMove()) {
//        transform->y                  = physics->floorY;
//        physics->dynamics.position.y  = physics->floorY;
//        physics->dynamics.velocity.y  = 0.0f;
//        physics->velY                 = 0.0f;
//        physics->onGround             = true;
//        physics->coyoteAvailable      = true;
//        physics->coyoteTimer          = physics->coyoteTime;
//    }
// 
//    // -- Coyote timer --
//    if (!physics->onGround) {
//        if (physics->coyoteTimer > 0.f) physics->coyoteTimer -= dt;
//        else                            physics->coyoteAvailable = false;
//    }
// 
//    // -- Jump buffer timer --
//    if (physics->jumpBufferTimer > 0.f) physics->jumpBufferTimer -= dt;
// 
//    // -- Ledge climb --
//    if (physics->ledgeClimb) {
//        float diff = physics->ledgeTargetY - transform->y;
//        if (std::abs(diff) < 0.05f) {
//            transform->y                 = physics->ledgeTargetY;
//            physics->dynamics.position.y = physics->ledgeTargetY;
//            physics->dynamics.velocity.y = 0.0f;
//            physics->velY                = 0.0f;
//            physics->onGround            = true;
//            physics->ledgeClimb          = false;
//            physics->coyoteAvailable     = true;
//            physics->coyoteTimer         = physics->coyoteTime;
//        }
//        else {
//            float step = physics->ledgeClimbSpeed * dt;
//            transform->y                 += std::min(step, diff);
//            physics->dynamics.position.y  = transform->y;
//            physics->dynamics.velocity.y  = 0.0f;
//            physics->velY                 = 0.0f;
//            physics->onGround             = false;
//        }
//    }
// 
//    // -- Jump input --
//    if (InputHandler::isKeyTriggered(GLFW_KEY_SPACE))
//        physics->jumpBufferTimer = physics->jumpBufferTime;
// 
//    // Jump if buffer active AND grounded or coyote available
//    if (physics->jumpBufferTimer > 0.f &&
//        (physics->onGround || physics->coyoteAvailable)) {
//        PlayerScript::jump(&obj);
//        physics->jumpBufferTimer  = 0.f;
//        physics->coyoteAvailable  = false;
//        messageBus.publish(Message("KeyPressed", nullptr, KeyEvent{ "SPACE", true }));
//    }
// 
//    // tap for small jump, hold for big jump
//    if (InputHandler::isKeyReleased(GLFW_KEY_SPACE)) {
//        if (physics->dynamics.velocity.y > 0.0f)
//            physics->dynamics.velocity.y *= 0.8f;
//    }
// 
//    // -- Movement input (A / D) --
//    float targetVelX = 0.0f;
//    if (InputHandler::isKeyHeld(GLFW_KEY_A)) {
//        physics->canMove = true;
//        targetVelX       = -physics->moveSpeed;
//        transform->flipX = true;
//        messageBus.publish(Message("KeyPressed", nullptr, KeyEvent{ "A", true }));
//    }
//    else if (InputHandler::isKeyHeld(GLFW_KEY_D) && physics->canMove) {
//        targetVelX       = physics->moveSpeed;
//        transform->flipX = false;
//        messageBus.publish(Message("KeyPressed", nullptr, KeyEvent{ "D", true }));
//    }
// 
//    // Flip to face mouse when not moving
//    if (!InputHandler::isKeyHeld(GLFW_KEY_A) && !InputHandler::isKeyHeld(GLFW_KEY_D)) {
//        Vector2D mouseWorld = InputHandler::getWorldMousePosition();
//        transform->flipX    = (mouseWorld.x < transform->x);
//    }
// 
//    // -- Integrate physics (damping + position) --
//    if (!obj.checkAutoMove() && !physics->ledgeClimb) {
//        if (CollisionInfo* col = obj.getComponent<CollisionInfo>()) {
//            PhysicsForces::applyDamping(physics, col->material.friction, targetVelX, dt);
//            PhysicsForces::updatePosition(transform, physics, dt);
//        }
//    }
// 
//    // -- Shoot input (left mouse held) --
//    if (!physics->canShoot) return;
// 
//    physics->shootTimer += dt;
// 
//    if (!InputHandler::isMouseLeftHeld()) {
//        physics->shootTimer = physics->fireRate; // ready to fire immediately on next press
//        return;
//    }
// 
//#ifdef _DEBUG
//    // In debug/editor mode, only shoot when scene window is hovered and not editing
//    // if (UISystem::isShowUI() &&
//    //     !(SceneWindow::isSceneHovered() && !EditorManager::isEditingMode()))
//    //     return;
//#endif
// 
//    // Check bullet count from visual logic global variable
//    if (!VisualLC::globalFloatVar.count("bullets") ||
//         VisualLC::globalFloatVar["bullets"] <= 0) {
//        DebugLog::addMessage("Out of bullets!", DebugMode::PlaySimul);
//        return;
//    }
// 
//    if (physics->shootTimer < physics->fireRate) return;
// 
//    GameObject* bullet = PlayerScript::findAvailableBullet(manager);
//    if (!bullet) return;
// 
//    // Find weapon spawn point (child with WeaponComponent)
//    Transform* spawnTransform = transform;
//    std::vector<GameObject*> allObjs;
//    manager.getAllGameObjects(allObjs);
// 
//    for (GameObject* candidate : allObjs) {
//        if (!candidate->hasComponent<WeaponComponent>()) continue;
//        if (!candidate->hasComponent<Transform>())       continue;
// 
//        Transform* wt = candidate->getComponent<Transform>();
//        if (wt->parentName != obj.getObjectName())       continue;
// 
//        spawnTransform = wt;
// 
//        if (!bullet->hasComponent<AudioComponent>()) {
//            physics->shootTimer = physics->fireRate * 0.5f;
//            DebugLog::addMessage("No audio on bullet!", DebugMode::PlaySimul);
//            break;
//        }
// 
//        Vector2D mouseWorldPos;
//#ifdef _DEBUG
//        // if (UISystem::isShowUI() && !EditorManager::isEditingMode()) {
//        //     ImVec2 scenePos, sceneSize;
//        //     EditorManager::getScenePosNSize(scenePos, sceneSize);
//        //     mouseWorldPos = InputHandler::getMousePositionInImGuiViewport(scenePos, sceneSize);
//        // }
//        // else {
//            mouseWorldPos = InputHandler::getWorldMousePosition();
//        //}
//#else
//        mouseWorldPos = InputHandler::getWorldMousePosition();
//#endif
// 
//        // play bullet audio
//        bullet->getComponent<AudioComponent>()->getDefaultChannel()->isPendingPlay = true;
// 
//        PlayerScript::shoot(bullet, spawnTransform, mouseWorldPos);
//        messageBus.publish(Message("KeyPressed", nullptr, KeyEvent{ "LEFT_CLICK", true }));
//        VisualLogicSystem::broadcastBulletShoot();
//        DebugLog::addMessage("Bullet fired!", DebugMode::PlaySimul);
//        physics->shootTimer = 0.0f;
//        break;
//    }
}
//
//void GameApp::onPhysicsUpdate(GameObjectManager& manager, float dt) {
//    std::vector<GameObject*> gameObjects;
//    manager.getAllGameObjects(gameObjects);
// 
//    for (GameObject* obj : gameObjects) {
//        const std::string& name = obj->getObjectName();
// 
//        if (name.find("bullet") != 0) continue;
// 
//        if (name.length() <= 6) continue;
//        bool numbered = true;
//        for (size_t i = 6; i < name.length(); ++i) {
//            if (!std::isdigit(static_cast<unsigned char>(name[i]))) {
//                numbered = false; break;
//            }
//        }
//        if (!numbered) continue;
// 
//        if (!obj->hasComponent<Physics>() || !obj->hasComponent<Transform>()) continue;
// 
//        Physics*   p = obj->getComponent<Physics>();
//        Transform* t = obj->getComponent<Transform>();
// 
//        if (p->alive) {
//            DynamicsSystem::UpdateForces(p->dynamics, dt);
// 
//            if (p->gravityDelayTimer > 0.0f) {
//                // Gravity delay - normal physics with gravity
//                p->gravityDelayTimer -= dt;
//                p->dynamics.velocity.y += -15.8f *
//                    (1.0f - (p->gravityDelayTimer / p->gravityDelay)) * dt;
// 
//                Vector3D deltaPos = Vec3_Scale(&p->dynamics.velocity, dt);
//                p->dynamics.position = Vec3_Add(&p->dynamics.position, &deltaPos);
//            }
//            else {
//                if (!p->onGround) {
//                    /*for water to move*/
//                    //flow force
//                    float flowDir      = (p->dynamics.velocity.x >= 0.0f) ? 1.0f : -1.0f;
//                    float flowStrength = 5.0f * expf(-p->lifeTimer * 0.6f);
//                    p->dynamics.velocity.x += flowStrength * flowDir * dt;
// 
//                    // strong gravity
//                    p->dynamics.velocity.y += -15.8f * dt;
// 
//                    //turbulence that decays over time
//                    float turbulence = sinf(p->lifeTimer * 8.0f) * 2.0f * expf(-p->lifeTimer);
//                    p->dynamics.velocity.y += turbulence * dt;
// 
//                    //quadratic drag
//                    float speed = sqrtf(p->dynamics.velocity.x * p->dynamics.velocity.x +
//                                        p->dynamics.velocity.y * p->dynamics.velocity.y);
//                    if (speed > 0.0001f) {
//                        const float drag = 0.5f * speed * speed;
//                        p->dynamics.velocity.x -= (p->dynamics.velocity.x / speed) * drag * dt;
//                        p->dynamics.velocity.y -= (p->dynamics.velocity.y / speed) * drag * dt;
//                    }
//                }
// 
//                Vector3D deltaPos = Vec3_Scale(&p->dynamics.velocity, dt);
//                p->dynamics.position = Vec3_Add(&p->dynamics.position, &deltaPos);
//            }
// 
//            t->x = p->dynamics.position.x;
//            t->y = p->dynamics.position.y;
//            p->lifeTimer += dt;
// 
//            //if (!EditorManager::isEditingMode()) {
//                if (p->lifeTimer > 2.0f) {
//                    float shrinkTimer = p->lifeTimer - 2.0f;
//                    float lifePercent = shrinkTimer / p->maxLifetime;
//                    float newScale    = std::max(p->initialScale * (1.0f - lifePercent), p->minScale);
//                    t->scaleX = newScale;
//                    t->scaleY = newScale;
// 
//                    if (obj->hasComponent<CollisionInfo>()) {
//                        CollisionInfo* col    = obj->getComponent<CollisionInfo>();
//                        col->colliderSize.x   = newScale;
//                        col->colliderSize.y   = newScale;
//                    }
//                }
//                // Deactivate when lifetime expires or falls off screen
//                if (p->lifeTimer >= p->maxLifetime || p->dynamics.position.y < -20.0f)
//                    PlayerScript::deactivateBullet(obj);
//            //}
//        }
//        else {
//            //if (!EditorManager::isEditingMode()) {
//                if (t->y > -100.0f || t->x > -100.0f) {
//                    t->x = p->dynamics.position.x = -1000.0f;
//                    t->y = p->dynamics.position.y = -1000.0f;
//                    p->dynamics.velocity.x = 0.0f;
//                    p->dynamics.velocity.y = 0.0f;
//                }
//            //}
//        }
//    }
//}

static void deactivateBullet(GameObject* bullet) {
    if (!bullet) return;
    Physics* p = bullet->getComponent<Physics>();
    Transform* t = bullet->getComponent<Transform>();
    if (!p || !t) return;
    if (!p->isOriginalStateSet) {
        p->originalPos = { t->x, t->y };
        p->isOriginalStateSet = true;
    }
    t->x = p->originalPos.x;
    t->y = p->originalPos.y;
    p->dynamics.position.x = p->originalPos.x;
    p->dynamics.position.y = p->originalPos.y;
    p->dynamics.velocity = { 0,0,0 };
    p->dynamics.forces.clear();
    p->lifeTimer = 0.0f;
    p->alive = false;
    if (bullet->hasComponent<Render>())
        bullet->getComponent<Render>()->clr.a = 0.0f;
}