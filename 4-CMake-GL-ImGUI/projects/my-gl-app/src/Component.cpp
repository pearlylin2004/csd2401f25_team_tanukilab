#include <iostream>
#include "Component.h"


class GameObject {
public:
    // Add components
    // Figure out templates later
    void addTransform(float x, float y) {
        transformComponent = std::make_unique<Transform>(x, y);
    }

    void addVelocity(float dx, float dy) {
        velocityComponent = std::make_unique<Velocity>(dx, dy);
    }

    void addHealth(float hp) {
        healthComponent = std::make_unique<Health>(hp);
    }

    void addRender(const char* sprite) {
        renderComponent = std::make_unique<Render>(sprite);
    }

    // Get components (returns nullptr if not present)
    Transform* getTransform() { return transformComponent.get(); }
    Velocity* getVelocity() { return velocityComponent.get(); }
    Health* getHealth() { return healthComponent.get(); }
	Render* getRender() { return renderComponent.get(); }

    // Convenience update methods
    void update(float deltaTime) {
        // Apply physics if has both transform and velocity
        if (transformComponent && velocityComponent) {
            transformComponent->x += velocityComponent->dx * deltaTime;
            transformComponent->y += velocityComponent->dy * deltaTime;
        }
    }

    void render() {
        if (transformComponent && renderComponent) {
            // Render at transform position
            std::cout << "Rendering at (" << transformComponent->x << ", " << transformComponent->y << ")\n";
        }
    }
private:
    // unique_ptr to store data
    std::unique_ptr<Transform> transformComponent;
    std::unique_ptr<Velocity> velocityComponent;
    std::unique_ptr<Health> healthComponent;
	std::unique_ptr<Render> renderComponent;
    // Add more as needed...
};