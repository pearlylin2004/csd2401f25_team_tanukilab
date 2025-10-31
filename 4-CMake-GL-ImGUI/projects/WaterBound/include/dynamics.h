#pragma once
#include "mathlib.h"
#include <vector>

struct Force
{
    Vector3D direction; // normalized
    float magnitude;
    float lifetime;     // in seconds (-1 for infinite)
    bool active = true;
};

struct Dynamics
{
    Vector3D position;
    Vector3D velocity;
    Vector3D acceleration;

    Vector3D angularVelocity;
    Vector3D angularAcceleration;

    float mass = 1.0f;
    float inverseMass = 1.0f;

    std::vector<Force> forces;
};

class DynamicsSystem
{
public:
    static void AddForce(Dynamics& dyn, const Vector3D& direction, float magnitude, float lifetime = -1.0f);
    static void UpdateForces(Dynamics& dyn, float deltaTime);
    static void Integrate(Dynamics& dyn, float deltaTime, const Vector3D& gravity = { 0.0f, -9.8f, 0.0f });
};
