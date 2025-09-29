#pragma once
#include <string>
#include <memory> //for unique_ptr
#include <renderer.h>


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
    std::string texID;
    renderer::model modelRef;
};

struct Physics : public Component {
    //idk add physics stuff here
public:
    float posX;
    float posY;
    float velY;
    float velX;

    bool inWaterTriggered; // true only when W is pressed

    // Physics(float x = 0.0f, float y = 0.0f);
   // Physics(float x = 0.0f, float y = 0.0f);

    bool inWater = false;
    bool buoancy = false;

    const float gravity = -2.0f;      // gravity pulling down
    const float waterBounce = 2.0f;   // upward velocity when entering water
    const float damping = 0.98f;       // slows motion gradually
    const float floorY = -8.0f;       // bottom limit
};

struct Input : public Component {
    //idk add input stuff here
public:
	Input(float speed = 0.0f) : moveSpeed(speed) {}

public:
    float moveSpeed = 0.0f;

};

enum class CollisionResponseMode {
    Stop,
    Bounce,
    Slide
};
struct CollisionInfo : public Component {
public:
    bool collided = false;
    float timeOfImpact = 0.0f;
    Vector2D normal{ 0,0 };
};