#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include "../Model Loading/mesh.h"

class GameObject
{
public:
    GameObject(Mesh* mesh);
    GameObject(Mesh* mesh, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale);

    void setPosition(const glm::vec3& pos);
    void setRotation(const glm::vec3& rot); // Euler angles in degrees
    void setScale(const glm::vec3& scale);
    void setScale(float uniformScale);

    glm::vec3 getPosition() const { return position; }
    glm::vec3 getRotation() const { return rotation; }
    glm::vec3 getScale() const { return scale; }

    glm::mat4 getModelMatrix() const;

    void draw(Shader& shader);
    Mesh* getMesh() { return mesh; }

private:
    Mesh* mesh;
    glm::vec3 position;
    glm::vec3 rotation; // Euler angles (x, y, z) in degrees
    glm::vec3 scale;

    void updateModelMatrix() const;
};