#pragma once
#include "mathlib.h"
#include "dynamics.h"
enum class ForceType {
    LINEAR,
    DRAG
};

struct Force {
    ForceType type;
    Vector3D direction;   // normalized for linear force
    float magnitude;  
    float lifetime;   // in seconds
    bool active = true;

    Force(ForceType t, const Vector3D& dir, float mag, float life = -1.0f)
        : type(t), direction(dir), magnitude(mag), lifetime(life) {}

    void age(float deltaTime) {
        if(lifetime > 0) {
            lifetime -= deltaTime;
            if(lifetime <= 0) active = false;
        }
    }
};
class PhysicsForces {
public:
    // Integrate physics for a Dynamics object over deltaTime
    void integrate(Dynamics& dyn, float deltaTime);
};