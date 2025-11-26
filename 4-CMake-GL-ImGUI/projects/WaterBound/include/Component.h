/* Start Header ************************************************************************/
/*!
\file        Component.h
\author      Seow Sin Le, s.sinle, 2401084, 20%
\author 	 Hugo Low Ren Hao, low.h, 2402272, 40%
\author 	 Pearly Lin Lee Ying, p.lin, 2401591, 15%
\author      Asha Mathyalakan, asha.m, 2402886, 25%
             Total - 100%
\par         s.sinle@digipen.edu
\par         low.h@digipen.edu
\par		 p.lin@digipen.edu
\par         asha.m@digipen.edu
\date        October, 1st, 2025
\brief       This file contains all the component definitions for the Component-based architecture.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <string>
#include <memory> //for unique_ptr
#include "renderer.h"
#include "mathlib.h"
#include "font.h"
#include "dynamics.h"
//#include "audio.h"


// basic component class
class Component {
public:
    // virtual just means that the child class can overwrite 
    // these functions
    virtual ~Component() = default;

	virtual std::unique_ptr<Component> clone() const = 0;
};

// Transform component to store position, rotation, and scale
struct Transform : public Component {
public:
	Transform() = default;
    Transform(float px, float py, float pz, float rot, float sx, float sy, float sz) 
        : x(px), y(py), z(pz), rotation(rot), scaleX(sx), scaleY(sy), scaleZ(sz), mdlWorld(1) {}

    std::unique_ptr<Component> clone() const override {
        return std::make_unique<Transform>(*this);
	}

    float x{ 0.f }, y{ 0.f }, z{ 0.f };
    float rotation = 0.0f;
    float scaleX = 1.0f, scaleY = 1.0f, scaleZ{ 0.f };
    bool flipX = false; // flip scaleX when change direction, true means flip to left
    glm::mat4 mdlWorld = 1.f;
};

// Render component to store rendering-related properties
struct Render : public Component {
public:
    Render() : texHDL{ 0 } {}

    std::unique_ptr<Component> clone() const override {
        return std::make_unique<Render>(*this);
    }

public:
    bool visible = true;
    bool hasTex = false;
    bool isTransparent = false;
    bool hasAnimation = false;
    //bool wireframe = false;
    // sorry but i need this for real time texture update
    // wait for boss sin le to help me settle this - kelly :)
    bool texChanged = false;
    GLuint texHDL;
    std::string texFile;
    glm::vec3 clr{ 1,1,1 };
    renderer::model modelRef;
};

struct FontComponent : public Component {
    FontComponent() = default;

    std::unique_ptr<Component> clone() const override {
        return std::make_unique<FontComponent>(*this);
    }

    //GLuint texHdl;
    Font::FontMdl mdl = Font::fontMdls[0];
    std::string word = "";
    float scale = 1;
    glm::vec3 clr{ 0.f,0.f,0.f };
    int fontType = 1;

    std::string fontPath = "assets/ARIAL.TTF"; 
};

// animation for different state
struct AnimateState {
    bool loop = true;
    Vector2D initialFrame{}, lastFrame{};
    int currentFrameColumn{}, currentFrameRow{}, totalColumn{ 1 }, totalRow{ 1 };
    float frameTime = 0.1f, frameTimer{};
    GLuint texHDL;
    std::string texFile;

    // do not serialize this
    bool texChanged = false;
};

struct Animation : public Component
{
    std::unique_ptr<Component> clone() const override {
        return std::make_unique<Animation>(*this);
    }

    bool runItBack = true;
    std::vector<AnimateState> animState; // to store the animation data for dff state, index by playerstate
};

// Physics component to store velocity and other physics-related properties
struct Physics : public Component {
public:
   
    std::unique_ptr<Component> clone() const override {
        return std::make_unique<Physics>(*this);
	}

    bool physicsFlag = true;
    float moveSpeed = 1.0f;

    Dynamics dynamics;

    bool canMove = true;  // Add this flag
    
  
    float velX = 0.0f;
    float velY = 0.0f;
    float gravity = -9.8f;

    //Jump 
    float damping = 0.98f;
    float jumpForce =100.0f;  
    float floorY = 0.0f;
    bool onGround = false;  

    //water bullet
    bool alive = false;
    float lifeTimer = 0.0f;     
    float maxLifetime = 3.0f;
    Vector2D originalPos{ 0, 0 };
    Vector2D originalVel{ 0, 0 };
    bool isOriginalStateSet = false;

    //bool inWaterTriggered; // true only when W is pressed

    bool inWater = false;
    bool buoancy = false;

    //const float waterBounce = 60.0f;   // upward velocity when entering water

};

// Input component to store input-related properties
struct Input : public Component {
public:
	//Input(float speed = 0.0f) : moveSpeed(speed) {}

    std::unique_ptr<Component> clone() const override {
        return std::make_unique<Input>(*this);
    }

public:
    //float moveSpeed = 0.0f;

};

// enum class used for CollisionSystem
enum class CollisionResponseMode {
    Stop
};

// CollisionInfo component to store collision-related information
struct CollisionInfo : public Component {
public:
                           
    std::unique_ptr<Component> clone() const override {
        return std::make_unique<CollisionInfo>(*this);
    }


    bool collisionFlag = true; // whether objest can collide with another obj
    shape colliderType = shape::square;
    Vector2D colliderSize{ 0,0 }; // size of the bounding box, if collider is circle then x will be diamater
    bool autoFitScale = true; // auto-fit the collider with the size of the mesh (i.e. obj bigger -> collider bigger)

    // run-time calculation, do not de/serialize
    bool collided = false; // check whether obj alr collided
    float timeOfImpact = 0.0f;
    Vector2D normal{ 0,0 };
    Vector2D contactPoint = { 0,0 };
    float penetration = 0.0f; //for it to not overlap 
};

// For imgui(testing)
struct UIComponent : public Component {
    //idk empty as of now
    //idk how i can use it
};

//for lua scripting
//to know which object to run
struct LuaScript : public Component {
public:
    LuaScript(const std::string& file)
        : filename(file) {
        // Extract just the filename without extension to use as scriptName
        size_t lastSlash = file.find_last_of("/\\");
        std::string fname = (lastSlash != std::string::npos) ? file.substr(lastSlash + 1) : file;

        size_t lastDot = fname.find_last_of('.');
        scriptName = (lastDot != std::string::npos) ? fname.substr(0, lastDot) : fname;
    }

    std::unique_ptr<Component> clone() const override {
        return std::make_unique<LuaScript>(*this);
    }

    std::string filename;   // Full path to the Lua file
    std::string scriptName; // Just the object name (e.g., "bullet", "player")
};

// Logic Container (FSM)
enum class PlayerState { Idle, Walking, Jumping, Falling, Shooting };

struct StateMachine : public Component {
    PlayerState state = PlayerState::Idle;
    float       stateTime = 0.f;
    bool        facingRight = true;
    std::unique_ptr<Component> clone() const override {
        return std::make_unique<StateMachine>(*this);
    }
};

//Audio component
struct FadeInfo {
    bool isFading = false;
    bool isFadingIn = false;
    float startVolume = 0.0f;
    float targetVolume = 0.0f;
    float fadeDuration = 0.0f;
    float fadeTimer = 0.0f;

    float defaultFadeInDuration = 1.0f;
    float defaultFadeOutDuration = 1.0f;
};

enum class AudioState {
    Stopped,
    Playing,
    Paused
};

//enum class soundID : int;
//enum class AudioState;
class FMOD::Channel;
struct AudioComponent : public Component {
public:
    AudioComponent() = default;

    AudioComponent(const std::string& file, float vol = 1.0f, bool shouldLoop = false, bool playStart = false)
        : audioFile(file)
        , volume(vol)
        , loop(shouldLoop)
        , playOnStart(playStart)
    {}

    std::unique_ptr<Component> clone() const override {
        return std::make_unique<AudioComponent>(*this);
    }

    // Audio ID - which sound to play
    std::string audioFile;
    //soundID soundId = static_cast<soundID>(0);

    // IMPORTANT: Each AudioComponent gets its own channel now
    // This is NOT managed by AudioHandler's channels vector
    FMOD::Channel* channel = nullptr;

    // Audio Settings
    float volume = 1.0f;
    float pitch = 1.0f;
    bool loop = false;
    bool playOnStart = false;
    bool muted = false;

    // Fade Settings (per-component)
    bool fadeInOnStart = false;
    float fadeInDuration = 1.0f;
    bool fadeOutOnStop = false;
    float fadeOutDuration = 1.0f;

    // Per-component fade state
    FadeInfo fadeInfo;
    /*bool isFading = false;
    bool isFadingIn = false;
    float fadeStartVolume = 0.0f;
    float fadeTargetVolume = 0.0f;
    float fadeDuration = 0.0f;
    float fadeTimer = 0.0f;*/

    // State Tracking
    AudioState state = AudioState::Stopped;
    bool isPendingPlay = false;
    bool isPendingStop = false;

    // Helper Methods
    bool isPlaying() const { return state == AudioState::Playing; }
    bool isPaused() const { return state == AudioState::Paused; }
    bool isStopped() const { return state == AudioState::Stopped; }

    void updateFades(float deltaTime) {
        if (!fadeInfo.isFading || !channel) return;

        fadeInfo.fadeTimer += deltaTime;

        float progress = fadeInfo.fadeTimer / fadeInfo.fadeDuration;
        if (progress > 1.0f) progress = 1.0f;

        float currentVolume;
        if (fadeInfo.isFadingIn) {
            currentVolume = fadeInfo.startVolume + (fadeInfo.targetVolume - fadeInfo.startVolume) * progress;
            std::cerr << "sound fading" << currentVolume << std::endl;
        }
        else {
            currentVolume = fadeInfo.startVolume + (fadeInfo.targetVolume - fadeInfo.startVolume) * progress;
        }

        FMODErrorCheck(channel->setVolume(currentVolume));

        if (progress >= 1.0f) {
            fadeInfo.isFading = false;

            if (!fadeInfo.isFadingIn && fadeInfo.targetVolume == 0.0f) {
                channel->stop();
                channel = nullptr;
            }
        }
    }
};
