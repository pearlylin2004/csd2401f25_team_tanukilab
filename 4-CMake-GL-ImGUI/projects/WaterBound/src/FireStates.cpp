#include "FireStates.h"
#include "GameObject.h"
#include "Component.h"
#include "Systems.h"

// ============================================================================
// FIRE ACTIVE STATE
// ============================================================================

void FireActiveState::onEnter(GameObject& obj) {
    // Ensure fire is visible and collidable
    if (CollisionInfo* c = obj.getComponent<CollisionInfo>()) {
        c->collisionFlag = true;
    }

    // Start fire crackling sound
    if (AudioComponent* audio = obj.getComponent<AudioComponent>()) {
        AudioChannel* fire = audio->getOrCreateChannel("fire_loop");
        if (fire) {
            fire->audioFile = "assets/audio/fire_crackle.wav";
            fire->loop = true;
            fire->volume = 0.5f;
            fire->isSpatial = true;  // 3D positional audio
            fire->isPendingPlay = true;
        }
    }

    DebugLog::addMessage("[FSM] " + obj.getObjectName() + " -> Fire Active", DebugMode::PlaySimul);
}

void FireActiveState::onUpdate(GameObject& obj, float dt) {
    (void)obj;
    (void)dt;
    // Fire animation handled by Animation component
}

// ============================================================================
// FIRE EXTINGUISHED STATE
// ============================================================================

void FireExtinguishedState::onEnter(GameObject& obj) {
    m_fadeTimer = 0.0f;

    // Disable collision - fire can't hurt player anymore
    if (CollisionInfo* c = obj.getComponent<CollisionInfo>()) {
        c->collisionFlag = false;
    }

    // Change to smoke/steam texture
    if (Render* r = obj.getComponent<Render>()) {
        r->texFile = "assets/textures/steam.png";
        r->texChanged = true;
    }

    // Stop fire sound, play steam hiss
    if (AudioComponent* audio = obj.getComponent<AudioComponent>()) {
        // Stop fire loop
        if (AudioChannel* fire = audio->getChannel("fire_loop")) {
            fire->isPendingStop = true;
            fire->fadeOutOnStop = true;
            fire->fadeOutDuration = 0.3f;
        }

        // Play steam sound
        AudioChannel* steam = audio->getOrCreateChannel("steam");
        if (steam) {
            steam->audioFile = "assets/audio/steam_hiss.wav";
            steam->loop = false;
            steam->volume = 0.6f;
            steam->isPendingPlay = true;
        }
    }

    DebugLog::addMessage("[FSM] " + obj.getObjectName() + " -> Fire Extinguished", DebugMode::PlaySimul);
}

void FireExtinguishedState::onUpdate(GameObject& obj, float dt) {
    // Optional: fade out smoke over time
    m_fadeTimer += dt;
    if (m_fadeTimer > 2.0f) {
        if (Render* r = obj.getComponent<Render>()) {
            r->clr.a = std::max(0.0f, 1.0f - (m_fadeTimer - 2.0f));
        }
    }
}