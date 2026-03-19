/* Start Header ************************************************************************/
/*!
\file       RefillStates.cpp
\author     Varick, v.teo, 2403417
\par        v.teo@digipen.edu
\date       February, 5th, 2026
\brief      Implements the Water Refill FSM states. Handles enter/update logic for
            Full and Empty states including collision, visual, and audio changes.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "RefillStates.h"
#include "GameObject.h"
#include "Component.h"
#include "Systems.h"

// ============================================================================
// REFILL FULL STATE
// ============================================================================

void RefillFullState::onEnter(GameObject& obj) {
    // Ensure refill is interactable
    if (CollisionInfo* c = obj.getComponent<CollisionInfo>()) {
        c->collisionFlag = true;
    }

    // Set full texture
    if (Render* r = obj.getComponent<Render>()) {
        r->texFile = "assets/textures/water_refill_full.png";
        r->texChanged = true;
    }

    DebugLog::addMessage("[FSM] " + obj.getObjectName() + " -> Refill Full", DebugMode::PlaySimul);
}

void RefillFullState::onUpdate(GameObject& obj, float dt) {
    (void)obj;
    (void)dt;
    // No per-frame logic needed for full state
}

// ============================================================================
// REFILL EMPTY STATE - One-time use optimization
// ============================================================================

void RefillEmptyState::onEnter(GameObject& obj) {
    // Disable collision - saves resources, can't be used again
    if (CollisionInfo* c = obj.getComponent<CollisionInfo>()) {
        c->collisionFlag = false;
    }

    // Disable physics if any
    if (Physics* p = obj.getComponent<Physics>()) {
        p->physicsFlag = false;
    }

    // Change to empty texture
    if (Render* r = obj.getComponent<Render>()) {
        r->texFile = "assets/textures/water_refill_empty.png";
        r->texChanged = true;
        r->clr.a = 0.5f;  // Semi-transparent to show it's used
    }

    // Play empty/used sound
    if (AudioComponent* audio = obj.getComponent<AudioComponent>()) {
        AudioChannel* used = audio->getOrCreateChannel("used");
        if (used) {
            used->audioFile = "assets/audio/refill_empty.wav";
            used->loop = false;
            used->isPendingPlay = true;
        }
    }

    DebugLog::addMessage("[FSM] " + obj.getObjectName() + " -> Refill Empty (collision disabled)", DebugMode::PlaySimul);
}

void RefillEmptyState::onUpdate(GameObject& obj, float dt) {
    (void)obj;
    (void)dt;
    // No per-frame logic needed for empty state
}