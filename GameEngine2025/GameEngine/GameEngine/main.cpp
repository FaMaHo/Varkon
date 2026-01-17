#include "Graphics\window.h"
#include "Camera\camera.h"
#include "Shaders\shader.h"
#include "Model Loading\mesh.h"
#include "Model Loading\texture.h"
#include "Model Loading\meshLoaderObj.h"
#include "Bullet\bullet.h"
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

glm::vec3 lightColor = glm::vec3(1.0f);
glm::vec3 lightPos = glm::vec3(0.0f, 500.0f, 0.0f);

// Crosshair VAO and VBO
GLuint crosshairVAO, crosshairVBO;

// Bullets
std::vector<Bullet> bullets;
bool lastMouseState = false;

// ================= FUNCTION DECLARATIONS =================
void processKeyboardInput();
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void drawCrosshair(int screenWidth, int screenHeight);

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

// ================= CURVED GROUND (from Nour's code) =================
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
    Shader bulletShader("Shaders/bullet_vertex_shader.glsl", "Shaders/bullet_fragment_shader.glsl");

    // ===== TEXTURES =====
    GLuint marsTexture = loadBMP("Resources/Textures/mars.bmp");
    GLuint baseTexture = loadBMP("Resources/Textures/Texture_1K/Base_BaseColor.bmp");
    GLuint headTexture = loadBMP("Resources/Textures/Texture_1K/Head_BaseColor.bmp");
    GLuint wingTexture = loadBMP("Resources/Textures/Texture_1K/Wing_BaseColor.bmp");
    GLuint inTexture = loadBMP("Resources/Textures/Texture_1K/In_BaseColor.bmp");
    GLuint gunTexture = loadBMP("Resources/Textures/SciFi_Gun_Full_Base/Paint_Base_Color.bmp");
    GLuint caveWallDiffuse = loadBMP("Resources/Textures/CaveWalls2_Base_Diffuse.bmp");
    GLuint asteroidDiffuse = loadBMP("Resources/Textures/Asteroid_1_Diffuse_1K.bmp");

    glEnable(GL_DEPTH_TEST);

    // Setup crosshair
    glGenVertexArrays(1, &crosshairVAO);
    glGenBuffers(1, &crosshairVBO);

    glBindVertexArray(crosshairVAO);
    glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ===== MESH CREATION =====
    Mesh stars = createStars(500, 2000.0f);
    Mesh ground = createGround(200.0f, marsTexture);

    MeshLoaderObj loader;

    // Spaceship
    std::vector<Texture> shipTextures;
    shipTextures.push_back({ baseTexture, "texture_diffuse" });
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

    // =============================== MAIN LOOP ===============================
    while (!window.isPressed(GLFW_KEY_ESCAPE) &&
        glfwWindowShouldClose(window.getWindow()) == 0)
    {
        window.clear();

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processKeyboardInput();

        // ===== SHOOTING MECHANIC =====
        bool currentMouseState = window.isMousePressed(GLFW_MOUSE_BUTTON_LEFT);
        if (currentMouseState && !lastMouseState)
        {
            glm::vec3 cameraPos = camera.getCameraPosition();
            glm::vec3 cameraDir = camera.getCameraViewDirection();
            glm::vec3 bulletStart = cameraPos + cameraDir * 2.0f;

            std::cout << "BULLET FIRED! Direction: (" << cameraDir.x << ", " << cameraDir.y << ", " << cameraDir.z << ")" << std::endl;

            Bullet newBullet(bulletStart, cameraDir, 300.0f, 3.0f);
            bullets.push_back(newBullet);
        }
        lastMouseState = currentMouseState;

        // Update bullets
        for (auto& bullet : bullets)
        {
            bullet.update(deltaTime);
        }

        // Remove inactive bullets
        bullets.erase(
            std::remove_if(bullets.begin(), bullets.end(),
                [](const Bullet& b) { return !b.isActive(); }),
            bullets.end()
        );

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

        glUniform3fv(glGetUniformLocation(shader.getId(), "lightColor"), 1, &lightColor[0]);
        glUniform3fv(glGetUniformLocation(shader.getId(), "lightPos"), 1, &lightPos[0]);
        glUniform3fv(glGetUniformLocation(shader.getId(), "viewPos"), 1,
            &camera.getCameraPosition()[0]);

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
        // Center ship
        glm::mat4 ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, -2.5f, -100.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.5f, 2.5f, 2.5f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        spaceship.draw(shader);

        // Right ship
        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(50.0f, -2.5f, -80.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.5f, 2.5f, 2.5f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        spaceship.draw(shader);

        // Left ship
        ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-50.0f, -2.5f, -80.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.5f, 2.5f, 2.5f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        spaceship.draw(shader);

        // ===== RENDER CAVE WALLS =====
        // Cave Wall Set - Large formation on the left
        ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-80.0f, -9.0f, -120.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.0f, 3.0f, 3.0f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        caveWallSet.draw(shader);

        // Cave Wall A - Close to player on right
        ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(40.0f, -8.0f, -260.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(-30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.5f, 2.5f, 2.5f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        caveWallA.draw(shader);

        // Cave Wall B - Behind ships
        ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-70.0f, -7.0f, -350.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(4.0f, 3.0f, 4.0f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        caveWallB.draw(shader);

        // Cave Wall C - Far left
        ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-100.0f, -6.0f, -480.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(120.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.5f, 3.5f, 3.5f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        caveWallC.draw(shader);

        // Additional Cave Wall A - Near center ship
        ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-30.0f, -8.5f, 400.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 2.0f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        caveWallA.draw(shader);

        // ===== RENDER ASTEROIDS =====
        // Asteroid 1 - Floating near right ship
        ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(60.0f, 5.0f, -85.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(35.0f), glm::vec3(1.0f, 0.5f, 0.3f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.5f, 3.5f, 3.5f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        asteroid.draw(shader);

        // Asteroid 2 - High above center ship
        ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-10.0f, 25.0f, -110.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(120.0f), glm::vec3(0.7f, 1.0f, 0.2f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(4.0f, 4.0f, 4.0f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        asteroid.draw(shader);

        // Asteroid 3 - Far left, distant
        ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-120.0f, 35.0f, -200.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(80.0f), glm::vec3(0.3f, 0.8f, 1.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(5.0f, 5.0f, 5.0f));
        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        asteroid.draw(shader);

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

        // ===== RENDER BULLETS =====
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        sunShader.use();
        for (auto& bullet : bullets)
        {
            bullet.draw(sunShader, ProjectionMatrix, ViewMatrix);
        }

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        // ===== DRAW CROSSHAIR =====
        drawCrosshair(window.getWidth(), window.getHeight());

        window.update();
    }

    return 0;
}

// ================= KEYBOARD INPUT =================
void processKeyboardInput()
{
    float speed = 30 * deltaTime;

    if (window.isPressed(GLFW_KEY_W)) camera.keyboardMoveFront(speed);
    if (window.isPressed(GLFW_KEY_S)) camera.keyboardMoveBack(speed);
    if (window.isPressed(GLFW_KEY_A)) camera.keyboardMoveLeft(speed);
    if (window.isPressed(GLFW_KEY_D)) camera.keyboardMoveRight(speed);
    if (window.isPressed(GLFW_KEY_R)) camera.keyboardMoveUp(speed);
    if (window.isPressed(GLFW_KEY_F)) camera.keyboardMoveDown(speed);
}

// ================= CROSSHAIR DRAWING =================
void drawCrosshair(int screenWidth, int screenHeight)
{
    GLboolean depthTestEnabled;
    glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, screenWidth, screenHeight, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glUseProgram(0);
    glColor3f(1.0f, 1.0f, 1.0f);

    float centerX = screenWidth / 2.0f;
    float centerY = screenHeight / 2.0f;
    float crosshairSize = 15.0f;
    float crosshairThickness = 2.0f;
    float crosshairGap = 5.0f;

    glLineWidth(crosshairThickness);
    glBegin(GL_LINES);

    // Horizontal line (left)
    glVertex2f(centerX - crosshairSize - crosshairGap, centerY);
    glVertex2f(centerX - crosshairGap, centerY);

    // Horizontal line (right)
    glVertex2f(centerX + crosshairGap, centerY);
    glVertex2f(centerX + crosshairSize + crosshairGap, centerY);

    // Vertical line (top)
    glVertex2f(centerX, centerY - crosshairSize - crosshairGap);
    glVertex2f(centerX, centerY - crosshairGap);

    // Vertical line (bottom)
    glVertex2f(centerX, centerY + crosshairGap);
    glVertex2f(centerX, centerY + crosshairSize + crosshairGap);

    glEnd();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    if (depthTestEnabled)
        glEnable(GL_DEPTH_TEST);
}