#include "collision.h"
#include <iostream>
#include <cmath>

CollisionSystem::CollisionSystem() {}
CollisionSystem::~CollisionSystem() {}

void CollisionSystem::addSphere(glm::vec3 center, float radius) {
    spheres.push_back(SphereCollider(center, radius));
    std::cout << "Added sphere at (" << center.x << ", " << center.y << ", " << center.z
        << ") with radius " << radius << std::endl;
}

void CollisionSystem::clear() {
    spheres.clear();
}

bool CollisionSystem::checkPlayerCollision(glm::vec3 playerPos, float playerRadius, glm::vec3& pushOut) {
    pushOut = glm::vec3(0.0f);
    bool hasCollision = false;

    for (const auto& sphere : spheres) {
        glm::vec3 localPush;
        if (sphereVsSphere(playerPos, playerRadius, sphere.center, sphere.radius, localPush)) {
            pushOut += localPush;
            hasCollision = true;

            std::cout << "COLLISION with sphere at (" << sphere.center.x << ", "
                << sphere.center.y << ", " << sphere.center.z << ")" << std::endl;
        }
    }

    return hasCollision;
}

bool CollisionSystem::sphereVsSphere(glm::vec3 pos1, float radius1, glm::vec3 pos2, float radius2, glm::vec3& pushOut) {
    // Calculate distance between centers
    glm::vec3 difference = pos1 - pos2;
    float distance = glm::length(difference);
    float minDistance = radius1 + radius2;

    // Check if spheres overlap
    if (distance < minDistance) {
        // Collision! Calculate push-out vector
        if (distance > 0.001f) {
            // Normal case: push away from other sphere
            glm::vec3 direction = glm::normalize(difference);
            float penetration = minDistance - distance;
            pushOut = direction * penetration * 1.1f;  // Extra 10% for safety
        }
        else {
            // Spheres are exactly on top of each other - push up
            pushOut = glm::vec3(0.0f, minDistance, 0.0f);
        }

        return true;
    }

    return false;
}