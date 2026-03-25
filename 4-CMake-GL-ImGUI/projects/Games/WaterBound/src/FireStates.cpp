/* Start Header ************************************************************************/
/*!
\file       FireStates.cpp
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date       February, 5th, 2026
\brief      Implements the Fire FSM states. Handles enter/update logic for Active
            and Extinguished states.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "FireStates.h"
#include "GameObject.h"
#include "Component.h"
#include "Systems.h"

// ============================================================================
// FIRE ACTIVE STATE
// ============================================================================

void FireActiveState::onEnter(GameObject& obj) {
    //// Ensure fire is visible and collidable
    //if (CollisionInfo* c = obj.getComponent<CollisionInfo>()) {
    //    c->collisionFlag = true;
    //    c->collisionTag = "fire";  // Ensure tag is "fire" for death logic
    //}

    //// Start fire crackling sound
    //if (AudioComponent* audio = obj.getComponent<AudioComponent>()) {
    //    AudioChannel* fire = audio->getOrCreateChannel("fire_loop");
    //    if (fire) {
    //        fire->audioFile = "assets/audio/fire_crackle.wav";
    //        fire->loop = true;
    //        fire->volume = 0.5f;
    //        fire->isSpatial = true;
    //        fire->isPendingPlay = true;
    //    }
    //}

    //DebugLog::addMessage("[FSM] " + obj.getObjectName() + " -> Fire Active", DebugMode::PlaySimul);
}

void FireActiveState::onUpdate(GameObject& obj, float dt) {
    /*(void)obj;
    (void)dt;*/
}

// ============================================================================
// FIRE EXTINGUISHED STATE
// ============================================================================

void FireExtinguishedState::onEnter(GameObject& obj) {
    //m_fadeTimer = 0.0f;

    //// Change collision tag to "steam" - keeps collision enabled for upward force
    //// but changes tag so death logic doesn't trigger
    //if (CollisionInfo* c = obj.getComponent<CollisionInfo>()) {
    //    c->collisionFlag = true;   // Keep collision ENABLED for upward force
    //    c->collisionTag = "steam"; // Change tag so player doesn't die
    //}

    //// Clear any pending collision events from when it was fire
    //if (VisualLC* vl = obj.getComponent<VisualLC>()) {
    //    vl->curCollision.clear();
    //}

    //// Change to smoke/steam texture
    //if (Render* r = obj.getComponent<Render>()) {
    //    r->texFile = "assets/textures/steam.png";
    //    r->texChanged = true;
    //}

    //// Stop fire sound, play steam hiss
    //if (AudioComponent* audio = obj.getComponent<AudioComponent>()) {
    //    if (AudioChannel* fire = audio->getChannel("fire_loop")) {
    //        fire->isPendingStop = true;
    //        fire->fadeOutOnStop = true;
    //        fire->fadeOutDuration = 0.3f;
    //    }

    //    AudioChannel* steam = audio->getOrCreateChannel("steam");
    //    if (steam) {
    //        steam->audioFile = "assets/audio/steam_hiss.wav";
    //        steam->loop = false;
    //        steam->volume = 0.6f;
    //        steam->isPendingPlay = true;
    //    }
    //}

    //DebugLog::addMessage("[FSM] " + obj.getObjectName() + " -> Fire Extinguished (now steam)", DebugMode::PlaySimul);
}

void FireExtinguishedState::onUpdate(GameObject& obj, float dt) {
    /*m_fadeTimer += dt;
    if (m_fadeTimer > 2.0f) {
        if (Render* r = obj.getComponent<Render>()) {
            r->clr.a = std::max(0.0f, 1.0f - (m_fadeTimer - 2.0f));
        }
    }*/
}