#pragma once

#include <glm.hpp>
#include <glew.h>
#include "..\Shaders\shader.h"

class Bullet
{
private:
	glm::vec3 position;
	glm::vec3 direction;
	float speed;
	float lifetime;
	float maxLifetime;
	bool active;

	// Rendering
	GLuint vao, vbo, ebo;
	void setupMesh();

public:
	Bullet();
	Bullet(glm::vec3 startPos, glm::vec3 dir, float bulletSpeed = 100.0f, float maxLife = 5.0f);
	~Bullet();

	void update(float deltaTime);
	void draw(Shader& shader, glm::mat4 projectionMatrix, glm::mat4 viewMatrix);

	bool isActive() const;
	glm::vec3 getPosition() const;
	glm::vec3 getDirection() const;

	void deactivate();
};