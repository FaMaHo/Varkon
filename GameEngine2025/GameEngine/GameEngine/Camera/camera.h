#pragma once

#include <glm.hpp>
#include <gtx\transform.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>
#include "..\Graphics\window.h"

class Camera
{
private:
    glm::vec3 cameraPosition;
    glm::vec3 cameraViewDirection;
    glm::vec3 cameraUp;
    glm::vec3 cameraRight;

    float yaw;
    float pitch;
    float mouseSensitivity;

    // Ground constraints
    float groundHeight;
    float eyeHeight;

    void updateCameraVectors();
    void clampToGround();

public:
    Camera();
    Camera(glm::vec3 cameraPosition);
    Camera(glm::vec3 cameraPosition, glm::vec3 cameraViewDirection, glm::vec3 cameraUp);
    ~Camera();

    glm::mat4 getViewMatrix();
    glm::vec3 getCameraPosition();
    glm::vec3 getCameraViewDirection();
    glm::vec3 getCameraUp();

    // Ground-safe movement
    void keyboardMoveFront(float cameraSpeed);
    void keyboardMoveBack(float cameraSpeed);
    void keyboardMoveLeft(float cameraSpeed);
    void keyboardMoveRight(float cameraSpeed);

    // Vertical movement
    void keyboardMoveUp(float cameraSpeed);
    void keyboardMoveDown(float cameraSpeed);

    // Mouse look
    void processMouseMovement(float xoffset, float yoffset);
    
    void setPosition(glm::vec3 newPosition);
    
};


