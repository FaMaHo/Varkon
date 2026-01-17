#include "Graphics\window.h"
#include "Camera\camera.h"
#include "Shaders\shader.h"
#include "Model Loading\mesh.h"
#include "Model Loading\texture.h"
#include "Model Loading\meshLoaderObj.h"

// ================= GLOBALS (from second code) =================
bool firstMouse = true;
float lastX = 1000.0f;
float lastY = 600.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

Window window("VARKON", 2000, 1200);
Camera camera;

glm::vec3 lightColor = glm::vec3(1.0f);
glm::vec3 lightPos = glm::vec3(0.0f, 500.0f, 0.0f);

// ================= STAR CREATION (same logic) =================
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

// ================= CURVED GROUND FROM FIRST CODE =================
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

// ================= MOUSE CALLBACK (unchanged) =================
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

void processKeyboardInput();

// =============================== MAIN ===============================
int main()
{
    glClearColor(0.02f, 0.05f, 0.15f, 1.0f);

    glfwSetCursorPosCallback(window.getWindow(), mouse_callback);
    glfwSetInputMode(window.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    Shader shader("Shaders/vertex_shader.glsl", "Shaders/fragment_shader.glsl");
    Shader sunShader("Shaders/sun_vertex_shader.glsl", "Shaders/sun_fragment_shader.glsl");

    GLuint marsTexture = loadBMP("Resources/Textures/mars.bmp");
    GLuint baseTexture = loadBMP("Resources/Textures/Texture_1K/Base_BaseColor.bmp");

    glEnable(GL_DEPTH_TEST);

    Mesh stars = createStars(500, 2000.0f);
    Mesh ground = createGround(200.0f, marsTexture);

    MeshLoaderObj loader;
    std::vector<Texture> shipTextures;
    shipTextures.push_back({ baseTexture, "texture_diffuse" });

    Mesh spaceship = loader.loadObj(
        "Resources/Models/Imperial_Steniel_obj.obj",
        shipTextures
    );

    while (!window.isPressed(GLFW_KEY_ESCAPE) &&
        glfwWindowShouldClose(window.getWindow()) == 0)
    {
        window.clear();

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processKeyboardInput();

        glm::mat4 ProjectionMatrix =
            glm::perspective(90.0f,
                window.getWidth() * 1.0f / window.getHeight(),
                0.1f, 10000.0f);

        glm::mat4 ViewMatrix =
            glm::lookAt(camera.getCameraPosition(),
                camera.getCameraPosition() + camera.getCameraViewDirection(),
                camera.getCameraUp());

        // ===== STARS =====
        sunShader.use();
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix;
        glUniformMatrix4fv(
            glGetUniformLocation(sunShader.getId(), "MVP"),
            1, GL_FALSE, &MVP[0][0]);

        glPointSize(2.0f);
        stars.drawPoints(sunShader);

        // ===== INFINITE GROUND (from first code) =====
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
                glm::mat4 ModelMatrix = glm::translate(
                    glm::mat4(1.0f),
                    glm::vec3((tileX + x) * tileSize, -10.0f,
                        (tileZ + z) * tileSize));

                MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

                glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
                glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);

                ground.draw(shader);
            }
        }

        // ===== SPACESHIP =====
        glm::mat4 ModelMatrix = glm::mat4(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, -9.0f, -50.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(180.0f), glm::vec3(0, 1, 0));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f));

        MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);

        spaceship.draw(shader);

        window.update();
    }
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
