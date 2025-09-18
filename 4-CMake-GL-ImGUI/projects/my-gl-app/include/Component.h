#pragma once

struct Transform {
    Transform(float a, float b) : x(a), y(b) {}

    float x, y;

};

struct Velocity {
    Velocity(float a, float b) : dx(a), dy(b) {}


    float dx, dy;
};

struct Render {
    Render(const char* sprite) : spritePath(sprite) {}
    const char* spritePath;
};

struct Health {
    Health(float health) : hp(health) {}

    float hp;
};