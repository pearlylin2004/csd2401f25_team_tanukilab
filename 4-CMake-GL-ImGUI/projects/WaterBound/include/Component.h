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
    glm::mat4 mdlWorld = 1.f;
};

// Render component to store rendering-related properties
struct Render : public Component {
public:
    Render() = default;

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
    Font::FontMdl mdl;
    std::string word = "";
    float scale = 1;
    glm::vec3 clr{ 0.f,0.f,0.f };
    int fontType = 1;

    std::string fontPath = "shaders/ARIAL.TTF"; 
};

struct Animation : public Component
{
    std::unique_ptr<Component> clone() const override {
        return std::make_unique<Animation>(*this);
    }

    bool loop = false;
    bool runItBack = true;
    GLuint texHDL;
    int currentFrameColumn{}, currentFrameRow{}, totalColumn{ 1 }, totalRow{ 1 };
    float frameTime = 0.1f, frameTimer{};
};

// Physics component to store velocity and other physics-related properties
struct Physics : public Component {
public:
   
    std::unique_ptr<Component> clone() const override {
        return std::make_unique<Physics>(*this);
	}

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

    bool alive = false;

    //water bullet
    Vector2D posBullet{};
    Vector2D velBullet{};
    bool aliveBullet{};

    //bool inWaterTriggered; // true only when W is pressed

    bool inWater = false;
    bool buoancy = false;

    //const float waterBounce = 60.0f;   // upward velocity when entering water

};

// Input component to store input-related properties
struct Input : public Component {
public:
	Input(float speed = 0.0f) : moveSpeed(speed) {}

    std::unique_ptr<Component> clone() const override {
        return std::make_unique<Input>(*this);
    }

public:
    float moveSpeed = 0.0f;

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

    bool collided = false;
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
        : filename(file)
    {
        //automatically determine table name from file path
        std::string name = file;
        size_t lastSlash = name.find_last_of("/\\");
        if (lastSlash != std::string::npos)
            name = name.substr(lastSlash + 1); //remove path
        size_t dot = name.find_last_of('.');
        if (dot != std::string::npos)
            name = name.substr(0, dot); //remove extension
        scriptName = name;
    }

    std::unique_ptr<Component> clone() const override {
        return std::make_unique<LuaScript>(*this);
    }

    std::string filename;   //path to the Lua file
    std::string scriptName; //Lua table name
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

