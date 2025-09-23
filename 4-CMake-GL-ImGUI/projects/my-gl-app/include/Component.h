#pragma once
#include <string>
#include <memory> //for unique_ptr


// basic component class
class Component {
public:
    // virtual just means that the child class can overwrite 
    // these functions
    virtual ~Component() = default;
};

struct Transform : public Component {
    Transform(float x, float y, float rot, float sx, float sy) 
        : x(x), y(y), rotation(rot), scaleX(sx), scaleY(sy) {}

    float x = 0.0f, y = 0.0f;
    float rotation = 0.0f;
    float scaleX = 1.0f, scaleY = 1.0f;
};

struct Velocity : public Component {
public:
    Velocity(float a, float b) : dx(a), dy(b) {}

public:
    float dx = 0.0f, dy = 0.0f;
};

struct Render : public Component {
    // idk add render stuff here
public:
    Render(const std::string& model = "square") : modelType(model) {}

public:
    std::string modelType = "square"; // "square", "circle", etc.

    bool visible = true;

};

struct Physics : public Component {
    //idk add physics stuff here
public:

};

struct Input : public Component {
    //idk add input stuff here
public:
	Input(float speed = 0.0f) : moveSpeed(speed) {}

public:
    float moveSpeed = 0.0f;

};