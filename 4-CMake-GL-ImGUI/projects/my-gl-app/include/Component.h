#pragma once



// basic component class
class Component {
public:
    virtual ~Component() = default;
    virtual void Update(float deltaTime) {}
    virtual void Render() {}
};

struct Transform : public Component {
public:
    Transform(float a = 0.0f, float b = 0.0f) : x(a), y(b) {}

public:
    float x, y;

};

struct Velocity : public Component {
public:
    Velocity(float a = 0.0f, float b = 0.0f) : dx(a), dy(b) {}

public:
    float dx, dy;
};

struct Render : public Component {
    // idk add render stuff here
public:
    Render(const std::string& model = "square") : modelType(model) {}

public:
    std::string modelType = "square"; // "square", "circle", etc.
    float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f; // Color
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