#include "dynamics.h"
#include "physics.h"

void PhysicsForces::integrate(Dynamics& dyn, float deltaTime) {
    // Update forces
    dyn.forcesManager.updateForces(deltaTime);

    // Sum forces and calculate acceleration
    Vector3D totalForce = dyn.forcesManager.sumLinearForces();
    Vector3D accelFromForce = Vec3_Scale(&totalForce, dyn.invMass); // totalForce * invMass
    dyn.acceleration = Vec3_Add(&accelFromForce, &dyn.gravity);    // + gravity

    // Integrate linear motion
    Vector3D velDelta = Vec3_Scale(&dyn.acceleration, deltaTime); // accel * deltaTime
    dyn.velocity = Vec3_Add(&dyn.velocity, &velDelta);            // velocity += accel * dt

    Vector3D posDelta = Vec3_Scale(&dyn.velocity, deltaTime);     // velocity * dt
    dyn.position = Vec3_Add(&dyn.position, &posDelta);            // position += velocity * dt

    // Integrate rotational motion
    float totalTorque = dyn.forcesManager.sumTorques();
    dyn.angularAcceleration = totalTorque * dyn.invInertiaMass;
    dyn.angularVelocity += dyn.angularAcceleration * deltaTime;
    dyn.angle += dyn.angularVelocity * deltaTime;
}
