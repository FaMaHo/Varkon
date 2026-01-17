#include "Graphics\window.h"
#include "Camera\camera.h"
#include "Shaders\shader.h"
#include "Model Loading\mesh.h"
#include "Model Loading\texture.h"
#include "Model Loading\meshLoaderObj.h"

Mesh createStars(int numStars, float spaceSize)
{
	std::vector<Vertex> vertices;
	std::vector<int> indices;

	srand(42); 

	for (int i = 0; i < numStars; i++)
	{
		Vertex star;

		float x = ((float)rand() / RAND_MAX - 0.5f) * spaceSize;
		float y = ((float)rand() / RAND_MAX - 0.5f) * spaceSize;
		float z = ((float)rand() / RAND_MAX - 0.5f) * spaceSize;

		star.pos = glm::vec3(x, y, z);
		star.normals = glm::vec3(0.0f, 1.0f, 0.0f);
		star.textureCoords = glm::vec2(0.0f, 0.0f);

		vertices.push_back(star);
		indices.push_back(i);
	}

	return Mesh(vertices, indices);
}


Mesh createGround(float size, GLuint textureId)
{
	std::vector<Vertex> vertices;
	std::vector<int> indices;

	// simple quad for the ground
	// Bottom left
	Vertex v0;
	v0.pos = glm::vec3(-size, 0.0f, -size);
	v0.normals = glm::vec3(0.0f, 1.0f, 0.0f);
	v0.textureCoords = glm::vec2(0.0f, 0.0f);
	vertices.push_back(v0);

	// Bottom right
	Vertex v1;
	v1.pos = glm::vec3(size, 0.0f, -size);
	v1.normals = glm::vec3(0.0f, 1.0f, 0.0f);
	v1.textureCoords = glm::vec2(10.0f, 0.0f); 
	vertices.push_back(v1);

	// Top right
	Vertex v2;
	v2.pos = glm::vec3(size, 0.0f, size);
	v2.normals = glm::vec3(0.0f, 1.0f, 0.0f);
	v2.textureCoords = glm::vec2(10.0f, 10.0f);
	vertices.push_back(v2);

	// Top left
	Vertex v3;
	v3.pos = glm::vec3(-size, 0.0f, size);
	v3.normals = glm::vec3(0.0f, 1.0f, 0.0f);
	v3.textureCoords = glm::vec2(0.0f, 10.0f);
	vertices.push_back(v3);

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);

	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(3);

	std::vector<Texture> textures;
	Texture tex;
	tex.id = textureId;
	tex.type = "texture_diffuse";
	textures.push_back(tex);

	return Mesh(vertices, indices, textures);
}

void processKeyboardInput();

float deltaTime = 0.0f;
float lastFrame = 0.0f;

Window window("VARKON", 2000, 1200);
Camera camera; 

glm::vec3 lightColor = glm::vec3(1.0f);
glm::vec3 lightPos = glm::vec3(0.0f, 500.0f, 0.0f); 

int main()
{
	glClearColor(0.02f, 0.05f, 0.15f, 1.0f); 

	
	Shader shader("Shaders/vertex_shader.glsl", "Shaders/fragment_shader.glsl");
	Shader sunShader("Shaders/sun_vertex_shader.glsl", "Shaders/sun_fragment_shader.glsl");

	
	GLuint marsTexture = loadBMP("Resources/Textures/mars.bmp");


	GLuint baseTexture = loadBMP("Resources/Textures/Texture_1K/Base_BaseColor.bmp");
	GLuint headTexture = loadBMP("Resources/Textures/Texture_1K/Head_BaseColor.bmp");
	GLuint wingTexture = loadBMP("Resources/Textures/Texture_1K/Wing_BaseColor.bmp");
	GLuint inTexture = loadBMP("Resources/Textures/Texture_1K/In_BaseColor.bmp");

	glEnable(GL_DEPTH_TEST);

	
	Mesh stars = createStars(500, 2000.0f);


	Mesh ground = createGround(200.0f, marsTexture);

	
	MeshLoaderObj loader;
	std::vector<Texture> shipTextures;
	Texture shipTex;
	shipTex.id = baseTexture; 
	shipTex.type = "texture_diffuse";
	shipTextures.push_back(shipTex);


	Mesh spaceship = loader.loadObj("Resources/Models/Imperial_Steniel_obj.obj", shipTextures);


	// Mesh spaceGun = loader.loadObj("Resources/Models/SPACESGUN.obj");


	while (!window.isPressed(GLFW_KEY_ESCAPE) &&
		glfwWindowShouldClose(window.getWindow()) == 0)
	{
		window.clear();
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processKeyboardInput();

	
		glm::mat4 ProjectionMatrix = glm::perspective(90.0f, window.getWidth() * 1.0f / window.getHeight(), 0.1f, 10000.0f);
		glm::mat4 ViewMatrix = glm::lookAt(camera.getCameraPosition(), camera.getCameraPosition() + camera.getCameraViewDirection(), camera.getCameraUp());

		// Render stars
		sunShader.use();
		GLuint MatrixID = glGetUniformLocation(sunShader.getId(), "MVP");

		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// Draw stars
		glPointSize(2.0f);
		stars.drawPoints(sunShader);

		// Render Mars
		shader.use();
		GLuint MatrixID2 = glGetUniformLocation(shader.getId(), "MVP");
		GLuint ModelMatrixID = glGetUniformLocation(shader.getId(), "model");

		// Position ground 
		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, -10.0f, 0.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightColor"), lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "viewPos"), camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		ground.draw(shader);

		// Render spaceship 
		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 20.0f, -50.0f));
		ModelMatrix = glm::rotate(ModelMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)); 
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		spaceship.draw(shader);

		window.update();
	}
}

void processKeyboardInput()
{
	float cameraSpeed = 30 * deltaTime;

	//translation
	if (window.isPressed(GLFW_KEY_W))
		camera.keyboardMoveFront(cameraSpeed);
	if (window.isPressed(GLFW_KEY_S))
		camera.keyboardMoveBack(cameraSpeed);
	if (window.isPressed(GLFW_KEY_A))
		camera.keyboardMoveLeft(cameraSpeed);
	if (window.isPressed(GLFW_KEY_D))
		camera.keyboardMoveRight(cameraSpeed);
	if (window.isPressed(GLFW_KEY_R))
		camera.keyboardMoveUp(cameraSpeed);
	if (window.isPressed(GLFW_KEY_F))
		camera.keyboardMoveDown(cameraSpeed);

	//rotation
	if (window.isPressed(GLFW_KEY_LEFT))
		camera.rotateOy(cameraSpeed);
	if (window.isPressed(GLFW_KEY_RIGHT))
		camera.rotateOy(-cameraSpeed);
	if (window.isPressed(GLFW_KEY_UP))
		camera.rotateOx(cameraSpeed);
	if (window.isPressed(GLFW_KEY_DOWN))
		camera.rotateOx(-cameraSpeed);
}