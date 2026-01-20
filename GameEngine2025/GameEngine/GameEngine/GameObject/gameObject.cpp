#include "gameObject.h"

GameObject::GameObject(Mesh* mesh)
    : mesh(mesh),
    position(0.0f, 0.0f, 0.0f),
    rotation(0.0f, 0.0f, 0.0f),
    scale(1.0f, 1.0f, 1.0f)
{
}

GameObject::GameObject(Mesh* mesh, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
    : mesh(mesh),
    position(position),
    rotation(rotation),
    scale(scale)
{
}

void GameObject::setPosition(const glm::vec3& pos)
{
    position = pos;
}

void GameObject::setRotation(const glm::vec3& rot)
{
    rotation = rot;
}

void GameObject::setScale(const glm::vec3& scaleVec)
{
    scale = scaleVec;
}

void GameObject::setScale(float uniformScale)
{
    scale = glm::vec3(uniformScale, uniformScale, uniformScale);
}

glm::mat4 GameObject::getModelMatrix() const
{
    glm::mat4 model = glm::mat4(1.0f);

    // Apply transformations: Translate -> Rotate -> Scale
    model = glm::translate(model, position);

    // Apply rotations in Y-X-Z order (common for game objects)
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    model = glm::scale(model, scale);

    return model;
}

void GameObject::draw(Shader& shader)
{
    if (mesh)
    {
        mesh->draw(shader);
    }
}