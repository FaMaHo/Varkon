#include "bullet.h"
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>
#include <iostream>

Bullet::Bullet()
{
	position = glm::vec3(0.0f);
	direction = glm::vec3(0.0f, 0.0f, -1.0f);
	speed = 100.0f;
	lifetime = 0.0f;
	maxLifetime = 5.0f;
	active = false;
}

Bullet::Bullet(glm::vec3 startPos, glm::vec3 dir, float bulletSpeed, float maxLife)
{
	position = startPos;
	direction = glm::normalize(dir);
	speed = bulletSpeed;
	lifetime = 0.0f;
	maxLifetime = maxLife;
	active = true;

	setupMesh();
}

Bullet::~Bullet()
{
	if (active)
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &ebo);
	}
}

void Bullet::setupMesh()
{
	// Create visible glowing projectile
	float width = 1.0f;   // Wider for visibility
	float length = 3.0f;  // Long tracer
	std::cout << "Bullet mesh created" << std::endl;
	float vertices[] = {
		// Front face
		-width, -width,  length,  0.0f, 0.0f, 1.0f,
		 width, -width,  length,  0.0f, 0.0f, 1.0f,
		 width,  width,  length,  0.0f, 0.0f, 1.0f,
		-width,  width,  length,  0.0f, 0.0f, 1.0f,
		// Back face
		-width, -width, -length,  0.0f, 0.0f, -1.0f,
		 width, -width, -length,  0.0f, 0.0f, -1.0f,
		 width,  width, -length,  0.0f, 0.0f, -1.0f,
		-width,  width, -length,  0.0f, 0.0f, -1.0f
	};

	unsigned int indices[] = {
		// Front face
		0, 1, 2, 2, 3, 0,
		// Back face
		4, 6, 5, 6, 4, 7,
		// Left face
		4, 0, 3, 3, 7, 4,
		// Right face
		1, 5, 6, 6, 2, 1,
		// Top face
		3, 2, 6, 6, 7, 3,
		// Bottom face
		4, 5, 1, 1, 0, 4
	};

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

void Bullet::update(float deltaTime)
{
	if (!active)
		return;

	// Move bullet forward
	position += direction * speed * deltaTime;

	// Update lifetime
	lifetime += deltaTime;

	// Deactivate if lifetime exceeded
	if (lifetime >= maxLifetime)
	{
		active = false;
	}
}

void Bullet::draw(Shader& shader, glm::mat4 projectionMatrix, glm::mat4 viewMatrix)
{
	if (!active)
		return;

	// Create model matrix for bullet position
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, position);

	// Rotate bullet to face its direction of travel
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 right = glm::normalize(glm::cross(up, direction));
	if (glm::length(right) < 0.01f) { // If direction is parallel to up
		right = glm::vec3(1.0f, 0.0f, 0.0f);
	}
	glm::vec3 newUp = glm::normalize(glm::cross(direction, right));

	glm::mat4 rotation = glm::mat4(1.0f);
	rotation[0] = glm::vec4(right, 0.0f);
	rotation[1] = glm::vec4(newUp, 0.0f);
	rotation[2] = glm::vec4(-direction, 0.0f);

	modelMatrix = modelMatrix * rotation;

	glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;

	glUniformMatrix4fv(glGetUniformLocation(shader.getId(), "MVP"), 1, GL_FALSE, &MVP[0][0]);

	// Draw the bullet
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

bool Bullet::isActive() const
{
	return active;
}

glm::vec3 Bullet::getPosition() const
{
	return position;
}

glm::vec3 Bullet::getDirection() const
{
	return direction;
}

void Bullet::deactivate()
{
	active = false;
}