#pragma once
#include <glm.hpp>
#include <vector>

// Simple sphere collider
struct SphereCollider {
    glm::vec3 center;
    float radius;

    SphereCollider(glm::vec3 c, float r) : center(c), radius(r) {}
};

class CollisionSystem {
private:
    std::vector<SphereCollider> spheres;

public:
    CollisionSystem();
    ~CollisionSystem();

    // Add a sphere collider
    void addSphere(glm::vec3 center, float radius);

    // Clear all colliders
    void clear();

    // Check if player sphere collides with any obstacle sphere
    // Returns true if collision, and provides push-out vector
    bool checkPlayerCollision(glm::vec3 playerPos, float playerRadius, glm::vec3& pushOut);

    // Get all spheres (for debug rendering)
    const std::vector<SphereCollider>& getSpheres() const { return spheres; }

private:
    // Check sphere vs sphere collision
    bool sphereVsSphere(glm::vec3 pos1, float radius1, glm::vec3 pos2, float radius2, glm::vec3& pushOut);
};