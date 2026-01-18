#include "Graphics\window.h"
#include "Camera\camera.h"
#include "Shaders\shader.h"
#include "Model Loading\mesh.h"
#include "Model Loading\texture.h"
#include "Model Loading\meshLoaderObj.h"
#include "Collisions\collision.h"
#include <vector>
#include <algorithm>
#include <iostream>

// ================= GLOBALS =================
bool firstMouse = true;
float lastX = 1000.0f;
float lastY = 600.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;


Window window("VARKON", 2000, 1200);
Camera camera;

CollisionSystem collisionSystem;
float playerRadius = 3.0f;  // Collision sphere radius for player
bool showDebugCollision = true;  // Toggle with a key

glm::vec3 lightColor = glm::vec3(1.0f);
glm::vec3 lightPos = glm::vec3(0.0f, 500.0f, 0.0f);

// ================= FUNCTION DECLARATIONS =================
void processKeyboardInput();
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// ================= STAR CREATION =================
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

// ================= CURVED GROUND =================
Mesh createGround(float size, GLuint textureId)
{
    std::vector<Vertex> vertices;
    std::vector<int> indices;

    int gridSize = 20;
    float step = (size * 2.0f) / gridSize;

    for (int z = 0; z <= gridSize; z++)
    {
        for (int x = 0; x <= gridSize; x++)
        {
            Vertex v;
            float xPos = -size + x * step;
            float zPos = -size + z * step;

            float dist = sqrt(xPos * xPos + zPos * zPos);
            float curvature = -dist * dist * 0.0001f;

            v.pos = glm::vec3(xPos, curvature, zPos);
            v.normals = glm::vec3(0.0f, 1.0f, 0.0f);
            v.textureCoords = glm::vec2(x * 2.0f, z * 2.0f);

            vertices.push_back(v);
        }
    }

    for (int z = 0; z < gridSize; z++)
    {
        for (int x = 0; x < gridSize; x++)
        {
            int tl = z * (gridSize + 1) + x;
            int tr = tl + 1;
            int bl = (z + 1) * (gridSize + 1) + x;
            int br = bl + 1;

            indices.push_back(tl);
            indices.push_back(bl);
            indices.push_back(tr);

            indices.push_back(tr);
            indices.push_back(bl);
            indices.push_back(br);
        }
    }

    std::vector<Texture> textures;
    Texture tex;
    tex.id = textureId;
    tex.type = "texture_diffuse";
    textures.push_back(tex);

    return Mesh(vertices, indices, textures);
}

// ================= MOUSE CALLBACK =================
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.processMouseMovement(xoffset, yoffset);
}

// Helper function to create approximate bounding box for objects
/*AABB createAABBForObject(glm::vec3 position, glm::vec3 scale, float baseSize = 10.0f) {
    glm::vec3 halfExtents = glm::vec3(baseSize) * scale * 0.5f;
    return AABB(position - halfExtents, position + halfExtents);
}*/

/// Add this NEW function to draw spheres
void drawDebugSphere(glm::vec3 center, float radius, Shader& shader, glm::mat4 ProjectionMatrix, glm::mat4 ViewMatrix) {
    const int segments = 16;
    const int rings = 8;
    std::vector<float> vertices;

    // Generate sphere wireframe
    for (int i = 0; i <= rings; i++) {
        float theta1 = i * 3.14159f / rings;
        float theta2 = (i + 1) * 3.14159f / rings;

        for (int j = 0; j <= segments; j++) {
            float phi = j * 2.0f * 3.14159f / segments;

            // First point
            float x1 = radius * sin(theta1) * cos(phi);
            float y1 = radius * cos(theta1);
            float z1 = radius * sin(theta1) * sin(phi);

            // Second point
            float x2 = radius * sin(theta2) * cos(phi);
            float y2 = radius * cos(theta2);
            float z2 = radius * sin(theta2) * sin(phi);

            vertices.push_back(center.x + x1);
            vertices.push_back(center.y + y1);
            vertices.push_back(center.z + z1);

            vertices.push_back(center.x + x2);
            vertices.push_back(center.y + y2);
            vertices.push_back(center.z + z2);
        }
    }

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    shader.use();
    glm::mat4 MVP = ProjectionMatrix * ViewMatrix;
    glUniformMatrix4fv(glGetUniformLocation(shader.getId(), "MVP"), 1, GL_FALSE, &MVP[0][0]);

    glDrawArrays(GL_LINES, 0, vertices.size() / 3);

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

// =============================== MAIN ===============================
int main()
{
    glClearColor(0.02f, 0.05f, 0.15f, 1.0f);

    // Setup mouse control
    glfwSetCursorPosCallback(window.getWindow(), mouse_callback);
    glfwSetInputMode(window.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // ===== SHADERS =====
    Shader shader("Shaders/vertex_shader.glsl", "Shaders/fragment_shader.glsl");
    Shader sunShader("Shaders/sun_vertex_shader.glsl", "Shaders/sun_fragment_shader.glsl");

    // ===== TEXTURES =====
    GLuint marsTexture = loadBMP("Resources/Textures/mars.bmp");

    // Spaceship Base textures
    GLuint baseTexture = loadBMP("Resources/Textures/Texture_1K/Base_BaseColor.bmp");
    GLuint baseNormal = loadBMP("Resources/Textures/Texture_1K/Base_Normal.bmp");

    GLuint gunTexture = loadBMP("Resources/Textures/SciFi_Gun_Full_Base/Paint_Base_Color.bmp");

    GLuint caveWallDiffuse = loadBMP("Resources/Textures/CaveWalls2_Base_Diffuse.bmp");

    GLuint asteroidDiffuse = loadBMP("Resources/Textures/Asteroid_1_Diffuse_1K.bmp");

    GLuint caveWall4Diffuse = loadBMP("Resources/Textures/CaveWalls4_Base_Diffuse.bmp");

    // Alien textures
    GLuint bodyTexture = loadBMP("Resources/Textures/body_Base_Color.bmp");
    GLuint eyeTexture = loadBMP("Resources/Textures/eye_Base_Color.bmp");

    glEnable(GL_DEPTH_TEST);

    // ===== MESH CREATION =====
    Mesh stars = createStars(500, 2000.0f);
    Mesh ground = createGround(200.0f, marsTexture);

    MeshLoaderObj loader;

    // Spaceship - using only base color and normal
    std::vector<Texture> shipTextures;
    shipTextures.push_back({ baseTexture, "texture_diffuse" });
    shipTextures.push_back({ baseNormal, "texture_normal" });

    Mesh spaceship = loader.loadObj("Resources/Models/Imperial_Steniel_obj.obj", shipTextures);

    // Space gun
    std::vector<Texture> gunTextures;
    gunTextures.push_back({ gunTexture, "texture_diffuse" });
    Mesh spaceGun = loader.loadObj("Resources/Models/SciFi_Gun_Full_Base.obj", gunTextures);

    // Cave walls
    std::vector<Texture> caveWallTextures;
    caveWallTextures.push_back({ caveWallDiffuse, "texture_diffuse" });
    Mesh caveWallA = loader.loadObj("Resources/Models/CaveWalls2_A.obj", caveWallTextures);
    Mesh caveWallB = loader.loadObj("Resources/Models/CaveWalls2_B.obj", caveWallTextures);
    Mesh caveWallC = loader.loadObj("Resources/Models/CaveWalls2_C.obj", caveWallTextures);
    Mesh caveWallSet = loader.loadObj("Resources/Models/CaveWalls2_Set.obj", caveWallTextures);

    // Asteroids
    std::vector<Texture> asteroidTextures;
    asteroidTextures.push_back({ asteroidDiffuse, "texture_diffuse" });
    Mesh asteroid = loader.loadObj("Resources/Models/Asteroid_1.obj", asteroidTextures);

    // CaveWalls4 Set - only using diffuse texture
    std::vector<Texture> caveWall4Textures;
    caveWall4Textures.push_back({ caveWall4Diffuse, "texture_diffuse" });
    Mesh caveWall4Set = loader.loadObj("Resources/Models/CaveWalls4_Set.obj", caveWall4Textures);

    // Rock04 models - using CaveWalls2 textures
    Mesh rock04A = loader.loadObj("Resources/Models/Rock04_A.obj", caveWallTextures);
    Mesh rock04B = loader.loadObj("Resources/Models/Rock04_B.obj", caveWallTextures);
    Mesh rock04C = loader.loadObj("Resources/Models/Rock04_C.obj", caveWallTextures);
    Mesh rock04D = loader.loadObj("Resources/Models/Rock04_D.obj", caveWall4Textures);
    Mesh rock04E = loader.loadObj("Resources/Models/Rock04_E.obj", caveWall4Textures);
    Mesh rock04Set = loader.loadObj("Resources/Models/Rock04_Set.obj", caveWallTextures);

    // Alien
    std::vector<Texture> bodyTextures;
    bodyTextures.push_back({ bodyTexture, "texture_diffuse" });
    Mesh alien = loader.loadObj("Resources/Models/body.obj", bodyTextures);

    // Add this section after all mesh creation and before the main loop:


    // ===== SETUP COLLISION SPHERES =====
    std::cout << "Setting up collision system with SPHERES..." << std::endl;

    // CaveWallSet at (-80, -9, -120)
    collisionSystem.addSphere(glm::vec3(-80.0f, -5.0f, -120.0f), 35.0f);

    // CaveWallA at (40, -8, -260)
    collisionSystem.addSphere(glm::vec3(40.0f, -5.0f, -260.0f), 30.0f);

    // CaveWallB at (-70, -7, -350)
    collisionSystem.addSphere(glm::vec3(-70.0f, -5.0f, -350.0f), 40.0f);

    // CaveWallC at (-100, -6, -480)
    collisionSystem.addSphere(glm::vec3(-100.0f, -5.0f, -480.0f), 35.0f);

    // CaveWallA at (-30, -8.5, 400)
    collisionSystem.addSphere(glm::vec3(-30.0f, -5.0f, 400.0f), 28.0f);

    // CaveWall4Set at (100, 10, 350)
    collisionSystem.addSphere(glm::vec3(100.0f, -3.0f, 350.0f), 35.0f);

    // Rock04 models - All the rocks
    collisionSystem.addSphere(glm::vec3(-150.0f, -5.0f, 200.0f), 25.0f);
    collisionSystem.addSphere(glm::vec3(-180.0f, -5.0f, -50.0f), 22.0f);
    collisionSystem.addSphere(glm::vec3(150.0f, -5.0f, -80.0f), 25.0f);
    collisionSystem.addSphere(glm::vec3(170.0f, -5.0f, 20.0f), 24.0f);
    collisionSystem.addSphere(glm::vec3(-160.0f, -5.0f, 80.0f), 22.0f);
    collisionSystem.addSphere(glm::vec3(140.0f, -5.0f, 50.0f), 20.0f);
    collisionSystem.addSphere(glm::vec3(-140.0f, -5.0f, 30.0f), 24.0f);
    collisionSystem.addSphere(glm::vec3(-175.0f, -5.0f, 150.0f), 26.0f);
    collisionSystem.addSphere(glm::vec3(-130.0f, -5.0f, -30.0f), 23.0f);
    collisionSystem.addSphere(glm::vec3(-155.0f, -5.0f, -120.0f), 21.0f);
    collisionSystem.addSphere(glm::vec3(-165.0f, -5.0f, 250.0f), 24.0f);

    std::cout << "Collision system ready with " << collisionSystem.getSpheres().size() << " spheres!" << std::endl;
    // =============================== MAIN LOOP ===============================
    while (!window.isPressed(GLFW_KEY_ESCAPE) &&
        glfwWindowShouldClose(window.getWindow()) == 0)
    {
        window.clear();

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processKeyboardInput();

  

        // ===== PROJECTION & VIEW MATRICES =====
        glm::mat4 ProjectionMatrix = glm::perspective(90.0f,
            window.getWidth() * 1.0f / window.getHeight(),
            0.1f, 10000.0f);

        glm::mat4 ViewMatrix = glm::lookAt(camera.getCameraPosition(),
            camera.getCameraPosition() + camera.getCameraViewDirection(),
            camera.getCameraUp());

        // ===== RENDER STARS =====
        sunShader.use();
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix;
        glUniformMatrix4fv(glGetUniformLocation(sunShader.getId(), "MVP"),
            1, GL_FALSE, &MVP[0][0]);

        glPointSize(2.0f);
        stars.drawPoints(sunShader);

        // ===== RENDER INFINITE GROUND =====
        shader.use();
        GLuint MatrixID = glGetUniformLocation(shader.getId(), "MVP");
        GLuint ModelID = glGetUniformLocation(shader.getId(), "model");

        glUniform1f(glGetUniformLocation(shader.getId(), "ambientStrength"), 0.2f);
        glUniform1f(glGetUniformLocation(shader.getId(), "specularStrength"), 0.5f);

        glUniform3fv(glGetUniformLocation(shader.getId(), "lightColor"), 1, &lightColor[0]);
        glUniform3fv(glGetUniformLocation(shader.getId(), "lightPos"), 1, &lightPos[0]);
        glUniform3fv(glGetUniformLocation(shader.getId(), "viewPos"), 1, &camera.getCameraPosition()[0]);

        float camX = camera.getCameraPosition().x;
        float camZ = camera.getCameraPosition().z;
        float tileSize = 400.0f;

        int tileX = (int)floor(camX / tileSize);
        int tileZ = (int)floor(camZ / tileSize);

        for (int x = -1; x <= 1; x++)
        {
            for (int z = -1; z <= 1; z++)
            {
                glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0f),
                    glm::vec3((tileX + x) * tileSize, -10.0f, (tileZ + z) * tileSize));

                MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
                glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
                glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);

                ground.draw(shader);
            }
        }

        // ===== RENDER SPACESHIPS =====
        // Make spaceships stand out with enhanced lighting
        glUniform1f(glGetUniformLocation(shader.getId(), "ambientStrength"), 0.7f);
        glUniform1f(glGetUniformLocation(shader.getId(), "specularStrength"), 1.5f);
        glm::vec3 brightLightColor = glm::vec3(2.2f, 2.2f, 2.2f);
        glUniform3fv(glGetUniformLocation(shader.getId(), "lightColor"), 1, &brightLightColor[0]);

        glm::mat4 ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-20.0f, -2.5f, -50.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.5f, 2.5f, 2.5f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        spaceship.draw(shader);

        // Reset to normal values after spaceships
        glUniform1f(glGetUniformLocation(shader.getId(), "ambientStrength"), 0.2f);
        glUniform1f(glGetUniformLocation(shader.getId(), "specularStrength"), 0.5f);
        glUniform3fv(glGetUniformLocation(shader.getId(), "lightColor"), 1, &lightColor[0]);

        // ===== RENDER CAVE WALLS =====
        glUniform1f(glGetUniformLocation(shader.getId(), "ambientStrength"), 0.1f);
        glUniform1f(glGetUniformLocation(shader.getId(), "specularStrength"), 0.3f);

        ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-80.0f, -9.0f, -120.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.0f, 3.0f, 3.0f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        caveWallSet.draw(shader);

        ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(40.0f, -8.0f, -260.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(-30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.5f, 2.5f, 2.5f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        caveWallA.draw(shader);

        ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-70.0f, -7.0f, -350.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(4.0f, 3.0f, 4.0f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        caveWallB.draw(shader);

        ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-100.0f, -6.0f, -480.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(120.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.5f, 3.5f, 3.5f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        caveWallC.draw(shader);

        ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-30.0f, -8.5f, 400.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 2.0f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        caveWallA.draw(shader);

        ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(100.0f, 10.0f, 350.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 2.0f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        caveWall4Set.draw(shader);

        // ===== RENDER ROCK04 MODELS =====
        ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-150.0f, -8.0f, 200.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.0f, 3.0f, 3.0f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        rock04A.draw(shader);

        ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-180.0f, -7.0f, -50.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(-30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.5f, 2.5f, 2.5f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        rock04B.draw(shader);

        ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(150.0f, -8.0f, -80.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(135.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.0f, 3.0f, 3.0f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        rock04C.draw(shader);

        ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(170.0f, -7.5f, 20.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(200.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.8f, 2.8f, 2.8f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        rock04D.draw(shader);

        ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-160.0f, -8.5f, 80.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(75.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.5f, 2.5f, 2.5f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        rock04E.draw(shader);

        ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(140.0f, -8.0f, 50.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(160.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 2.0f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        rock04Set.draw(shader);

        // ===== ADDITIONAL ROCKS FOR LEFT SIDE BALANCE =====
        ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-140.0f, -8.0f, 30.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.8f, 2.8f, 2.8f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        rock04C.draw(shader);

        ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-175.0f, -7.8f, 150.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.2f, 3.2f, 3.2f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        rock04D.draw(shader);

        ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-130.0f, -8.2f, -30.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(110.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.6f, 2.6f, 2.6f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        rock04A.draw(shader);

        ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-155.0f, -7.5f, -120.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(-80.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.2f, 2.2f, 2.2f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        rock04Set.draw(shader);

        ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-165.0f, -8.3f, 250.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(25.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.9f, 2.9f, 2.9f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        rock04B.draw(shader);

        glUniform1f(glGetUniformLocation(shader.getId(), "ambientStrength"), 0.2f);
        glUniform1f(glGetUniformLocation(shader.getId(), "specularStrength"), 0.5f);

        // ===== RENDER ALIENS =====
        glUniform1f(glGetUniformLocation(shader.getId(), "ambientStrength"), 0.7f);
        glUniform1f(glGetUniformLocation(shader.getId(), "specularStrength"), 1.5f);
        glUniform3fv(glGetUniformLocation(shader.getId(), "lightColor"), 1, &brightLightColor[0]);
        // Alien 1
        ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(15.0f, -8.0f, -50.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.5f, 1.5f, 1.5f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        alien.draw(shader);

        glUniform1f(glGetUniformLocation(shader.getId(), "ambientStrength"), 0.2f);
        glUniform1f(glGetUniformLocation(shader.getId(), "specularStrength"), 0.5f);
        glUniform3fv(glGetUniformLocation(shader.getId(), "lightColor"), 1, &lightColor[0]);

        // ===== RENDER ASTEROIDS =====
        glUniform1f(glGetUniformLocation(shader.getId(), "ambientStrength"), 0.7f);
        glUniform1f(glGetUniformLocation(shader.getId(), "specularStrength"), 1.5f);
        glUniform3fv(glGetUniformLocation(shader.getId(), "lightColor"), 1, &brightLightColor[0]);

        // Astroid 1
        ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(15.0f, 50.0f, -50.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(35.0f), glm::vec3(1.0f, 0.5f, 0.3f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(7.0f, 7.0f, 7.0f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        asteroid.draw(shader);

        glUniform1f(glGetUniformLocation(shader.getId(), "ambientStrength"), 0.2f);
        glUniform1f(glGetUniformLocation(shader.getId(), "specularStrength"), 0.5f);
        glUniform3fv(glGetUniformLocation(shader.getId(), "lightColor"), 1, &lightColor[0]);

        // ===== RENDER SPACE GUN (First-Person Weapon) =====
        glm::vec3 gunOffset = glm::vec3(0.0f, -0.1f, 0.2f);
        glm::vec3 cameraRight = glm::normalize(glm::cross(camera.getCameraViewDirection(), camera.getCameraUp()));
        glm::vec3 gunPos = camera.getCameraPosition()
            + camera.getCameraViewDirection() * gunOffset.z
            + cameraRight * gunOffset.x
            + camera.getCameraUp() * gunOffset.y;

        ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, gunPos);

        glm::vec3 forward = glm::normalize(camera.getCameraViewDirection());
        glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::normalize(glm::cross(worldUp, forward));
        glm::vec3 up = glm::normalize(glm::cross(forward, right));

        glm::mat4 cameraAlign = glm::mat4(1.0f);
        cameraAlign[0] = glm::vec4(right, 0.0f);
        cameraAlign[1] = glm::vec4(up, 0.0f);
        cameraAlign[2] = glm::vec4(-forward, 0.0f);
        cameraAlign[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

        ModelMatrix = ModelMatrix * cameraAlign;
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(-70.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(8.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(-5.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.3f, 0.3f, 0.3f));

        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        spaceGun.draw(shader);

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        // ===== DEBUG: RENDER COLLISION BOXES =====
        // ===== DEBUG: RENDER COLLISION SPHERES =====
        if (showDebugCollision) {
            glDisable(GL_DEPTH_TEST);
            sunShader.use();

            // Draw all obstacle spheres
            for (const auto& sphere : collisionSystem.getSpheres()) {
                drawDebugSphere(sphere.center, sphere.radius, sunShader, ProjectionMatrix, ViewMatrix);
            }

            // Draw player sphere (in a different color - we'll make it green by drawing it twice)
            glm::vec3 playerPos = camera.getCameraPosition();
            drawDebugSphere(playerPos, playerRadius, sunShader, ProjectionMatrix, ViewMatrix);

            glEnable(GL_DEPTH_TEST);
        }
        window.update();
    }

    return 0;
}

// ================= KEYBOARD INPUT =================
void processKeyboardInput()
{
    float speed = 30 * deltaTime;

    // Get current position
    glm::vec3 currentPos = camera.getCameraPosition();
    glm::vec3 desiredPos = currentPos;

    // Calculate movement directions
    glm::vec3 forward = camera.getCameraViewDirection();
    forward.y = 0.0f;
    if (glm::length(forward) > 0.001f) {
        forward = glm::normalize(forward);
    }

    glm::vec3 right = glm::normalize(glm::cross(camera.getCameraViewDirection(), camera.getCameraUp()));
    right.y = 0.0f;
    if (glm::length(right) > 0.001f) {
        right = glm::normalize(right);
    }

    // Apply input to desired position
    if (window.isPressed(GLFW_KEY_W)) {
        desiredPos += forward * speed;
    }
    if (window.isPressed(GLFW_KEY_S)) {
        desiredPos -= forward * speed;
    }
    if (window.isPressed(GLFW_KEY_A)) {
        desiredPos -= right * speed;
    }
    if (window.isPressed(GLFW_KEY_D)) {
        desiredPos += right * speed;
    }
    if (window.isPressed(GLFW_KEY_R)) {
        desiredPos.y += speed;
    }
    if (window.isPressed(GLFW_KEY_F)) {
        desiredPos.y -= speed;
    }

    // Collision detection with iterative resolution
    glm::vec3 finalPos = desiredPos;

    const int MAX_ITERATIONS = 5;
    for (int i = 0; i < MAX_ITERATIONS; i++) {
        glm::vec3 pushOut;
        bool hasCollision = collisionSystem.checkPlayerCollision(finalPos, playerRadius, pushOut);

        if (hasCollision) {
            finalPos += pushOut;
            std::cout << "Iteration " << i << " - Pushed by ("
                << pushOut.x << ", " << pushOut.y << ", " << pushOut.z << ")" << std::endl;
        }
        else {
            break;
        }
    }

    // Final safety check
    glm::vec3 testPush;
    if (collisionSystem.checkPlayerCollision(finalPos, playerRadius, testPush)) {
        std::cout << "!!! BLOCKING MOVEMENT - STILL IN COLLISION !!!" << std::endl;
        finalPos = currentPos;
    }

    // Set position
    camera.setPosition(finalPos);

    // Toggle debug visualization
    static bool toggleKeyPressed = false;
    if (window.isPressed(GLFW_KEY_T) && !toggleKeyPressed) {
        showDebugCollision = !showDebugCollision;
        std::cout << "Debug collision: " << (showDebugCollision ? "ON" : "OFF") << std::endl;
        toggleKeyPressed = true;
    }
    if (!window.isPressed(GLFW_KEY_T)) {
        toggleKeyPressed = false;
    }
}