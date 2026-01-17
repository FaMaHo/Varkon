#include "Plasma3D.h"

Plasma3D::Plasma3D(glm::vec3 pos, glm::vec3 vel, float rad)
    : position(pos), velocity(vel), active(true), lifetime(5.0f), radius(rad)
{
}

void Plasma3D::update(float deltaTime)
{
    if (!active) return;

    position += velocity * deltaTime;
    lifetime -= deltaTime;

    // Deactivate if lifetime expired or out of bounds
    if (lifetime <= 0.0f)
    {
        active = false;
    }

    // Optional: deactivate if too far from origin
    float distanceFromOrigin = glm::length(position);
    if (distanceFromOrigin > 1000.0f)
    {
        active = false;
    }
}