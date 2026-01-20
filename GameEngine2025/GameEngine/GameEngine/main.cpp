#include "Graphics/window.h"
#include "Camera/camera.h"
#include "Shaders/shader.h"
#include "ResourceManager/resourceManager.h"
#include "SceneManager/sceneManager.h"
#include <iostream>

// ================= GLOBALS =================
bool firstMouse = true;
float lastX = 1000.0f;
float lastY = 600.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

Window window("VARKON", 2000, 1200);
Camera camera;

// Scene management
int currentScene = 1;
bool key1Pressed = false;
bool key2Pressed = false;
bool key3Pressed = false;
bool keyNPressed = false;

// Message display control
bool messagePrinted = false;

// ================= FUNCTION DECLARATIONS =================
void processKeyboardInput(SceneManager& sceneManager);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

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

    // SHADERS
    Shader shader("Shaders/vertex_shader.glsl", "Shaders/fragment_shader.glsl");
    Shader sunShader("Shaders/sun_vertex_shader.glsl", "Shaders/sun_fragment_shader.glsl");

    glEnable(GL_DEPTH_TEST);

    // INITIALIZE SCENE MANAGER
    SceneManager sceneManager;
    sceneManager.initializeResources();  // Load all resources once
    sceneManager.loadScene(1);            // Start with scene 1

    std::cout << "\n========================================" << std::endl;
    std::cout << "Game Controls:" << std::endl;
    std::cout << "  WASD - Move" << std::endl;
    std::cout << "  R/F - Move up/down" << std::endl;
    std::cout << "  Mouse - Look around" << std::endl;
    std::cout << "  ESC - Quit" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // =============================== MAIN LOOP ===============================
    while (!window.isPressed(GLFW_KEY_ESCAPE) &&
        glfwWindowShouldClose(window.getWindow()) == 0)
    {
        window.clear();

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Check proximity triggers EVERY frame
        sceneManager.checkProximityTriggers(camera.getCameraPosition());

        // Display message if near trigger
        std::string triggerMsg = sceneManager.getTriggerMessage();
        if (!triggerMsg.empty())
        {
            if (!messagePrinted)
            {
                std::cout << "\n>>> " << triggerMsg << " <<<\n" << std::endl;
                messagePrinted = true;
            }
        }
        else
        {
            messagePrinted = false;
        }

        processKeyboardInput(sceneManager);

        // ===== PROJECTION & VIEW MATRICES =====
        glm::mat4 ProjectionMatrix = glm::perspective(90.0f,
            window.getWidth() * 1.0f / window.getHeight(),
            0.1f, 10000.0f);

        glm::mat4 ViewMatrix = camera.getViewMatrix();

        // ===== RENDER SCENE =====
        sceneManager.renderStars(ProjectionMatrix, ViewMatrix, sunShader);
        sceneManager.renderGround(ProjectionMatrix, ViewMatrix, camera.getCameraPosition(), shader);
        sceneManager.render(ProjectionMatrix, ViewMatrix, camera.getCameraPosition(), shader);

        window.update();
    }

    return 0;
}

// ================= KEYBOARD INPUT =================
void processKeyboardInput(SceneManager& sceneManager)
{
    float speed = 30 * deltaTime;

    // Movement
    if (window.isPressed(GLFW_KEY_W)) camera.keyboardMoveFront(speed);
    if (window.isPressed(GLFW_KEY_S)) camera.keyboardMoveBack(speed);
    if (window.isPressed(GLFW_KEY_A)) camera.keyboardMoveLeft(speed);
    if (window.isPressed(GLFW_KEY_D)) camera.keyboardMoveRight(speed);
    if (window.isPressed(GLFW_KEY_R)) camera.keyboardMoveUp(speed);
    if (window.isPressed(GLFW_KEY_F)) camera.keyboardMoveDown(speed);

    // Check for 'N' key to trigger scene transition
    if (window.isPressed(GLFW_KEY_N) && !keyNPressed)
    {
        keyNPressed = true;

        // Check if near a trigger
        int trigger = sceneManager.getNearbyTrigger();
        if (trigger >= 0)
        {
            // Get target scene from trigger
            const std::vector<TriggerZone>& zones = sceneManager.getTriggerZones();
            int targetScene = zones[trigger].targetScene;
            std::cout << "\n=== Entering Portal - Transitioning to Scene " << targetScene << " ===\n";
            sceneManager.loadScene(targetScene);
        }
    }
    else if (!window.isPressed(GLFW_KEY_N))
    {
        keyNPressed = false;
    }
}