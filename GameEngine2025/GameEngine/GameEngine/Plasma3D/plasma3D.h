#pragma once

#include <glm.hpp>
#include <vector>

class Plasma3D
{
public:
    glm::vec3 position;
    glm::vec3 velocity;
    bool active;
    float lifetime;
    float radius;

    Plasma3D(glm::vec3 pos, glm::vec3 vel, float rad = 0.5f);
    void update(float deltaTime);
    bool isActive() const { return active; }
};