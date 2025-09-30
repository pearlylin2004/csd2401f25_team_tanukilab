/* Start Header ************************************************************************/
/*!
\file		Component.h
\author
\par
\date		Month, Day, 2025
\brief
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

struct Transform : public Component {
public:
	Transform() = default;
    Transform(float x, float y, float rot, float sx, float sy) 
        : x(x), y(y), rotation(rot), scaleX(sx), scaleY(sy) {}

    float x{ 0.f }, y{ 0.f }, z{ 0.f };
    float rotation = 0.0f;
    float scaleX = 1.0f, scaleY = 1.0f, scaleZ{ 0.f };
    glm::mat4 mdlWorld;
};

struct Velocity : public Component {
public:
	Velocity() = default;
    Velocity(float a, float b) : dx(a), dy(b) {}

public:
    float dx = 0.0f, dy = 0.0f;
};

struct Render : public Component {
    // idk add render stuff here
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

struct Physics : public Component {
    //idk add physics stuff here
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

struct Input : public Component {
    //idk add input stuff here
public:
	Input(float speed = 0.0f) : moveSpeed(speed) {}

public:
    float moveSpeed = 0.0f;

};

//for collision system
enum class CollisionResponseMode {
    Stop
};
struct CollisionInfo : public Component {
public:
    bool collided = false;
    float timeOfImpact = 0.0f;
    Vector2D normal{ 0,0 };
    Vector2D contactPoint = { 0,0 };
};

//for imgui(testing)
struct UIComponent : public Component {
    //idk empty as of now
    //idk how i can use it
};