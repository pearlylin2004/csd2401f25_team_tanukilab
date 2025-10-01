/* Start Header ************************************************************************/
/*!
\file        Component.h
\author      Seow Sin Le, s.sinle, 2401084, 20%
\author 	 Hugo Low Ren Hao, low.h, 2402272, 20%
\author 	 Pearly Lin Lee Ying, p.lin, 2401591, 10%
\par         s.sinle@digipen.edu
\par         low.h@digipen.edu
\par		 p.lin@digipen.edu
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
#include <renderer.h>
#include <mathlib.h>


// basic component class
class Component {
public:
    // virtual just means that the child class can overwrite 
    // these functions
    virtual ~Component() = default;
};

// Transform component to store position, rotation, and scale
struct Transform : public Component {
public:
	Transform() = default;
    Transform(float px, float py, float pz, float rot, float sx, float sy, float sz) 
        : x(px), y(py), z(pz), rotation(rot), scaleX(sx), scaleY(sy), scaleZ(sz), mdlWorld(1) {}

    float x{ 0.f }, y{ 0.f }, z{ 0.f };
    float rotation = 0.0f;
    float scaleX = 1.0f, scaleY = 1.0f, scaleZ{ 0.f };
    glm::mat4 mdlWorld = 1.f;
};

// Render component to store rendering-related properties
struct Render : public Component {
public:
    Render() = default;

public:
    bool visible = true;
    bool hasTex = false;
    bool isTransparent = false;
    bool wireframe = false;
    GLuint texHDL;
    std::string texFile;
    
    renderer::model modelRef;
};

// Physics component to store velocity and other physics-related properties
struct Physics : public Component {
public:
   
    float velX = 0.0f;
    float velY = 0.0f;
    float gravity = -9.8f;

    //Jump 
    float damping = 0.99f;
    float jumpForce = 10.0f;  
    float floorY = 0.0f;
    bool onGround = true;  
    float Speed = 150.0f;

    //water bullet
    Vector2D posBullet;
    Vector2D velBullet;
    bool aliveBullet;

    //bool inWaterTriggered; // true only when W is pressed

    bool inWater = false;
    bool buoancy = false;

    //const float waterBounce = 60.0f;   // upward velocity when entering water

};

// Input component to store input-related properties
struct Input : public Component {
public:
	Input(float speed = 0.0f) : moveSpeed(speed) {}

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
    bool collided = false;
    float timeOfImpact = 0.0f;
    Vector2D normal{ 0,0 };
    Vector2D contactPoint = { 0,0 };
};

// For imgui(testing)
struct UIComponent : public Component {
    //idk empty as of now
    //idk how i can use it
};