#include "camera.h"

Camera::Camera()
{
    cameraPosition = glm::vec3(0.0f, 5.0f, 0.0f);
    cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    yaw = -90.0f;
    pitch = 0.0f;
    mouseSensitivity = 0.1f;

    groundHeight = -10.0f; // same as your ground Y
    eyeHeight = 1.7f;      // FPS-style eye height

    updateCameraVectors();
    clampToGround();
}

Camera::Camera(glm::vec3 position)
{
    cameraPosition = position;
    cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    yaw = -90.0f;
    pitch = 0.0f;
    mouseSensitivity = 0.1f;

    groundHeight = -10.0f;
    eyeHeight = 1.7f;

    updateCameraVectors();
    clampToGround();
}

Camera::Camera(glm::vec3 position, glm::vec3 direction, glm::vec3 up)
{
    cameraPosition = position;
    cameraViewDirection = glm::normalize(direction);
    cameraUp = up;

    yaw = -90.0f;
    pitch = 0.0f;
    mouseSensitivity = 0.1f;

    groundHeight = -10.0f;
    eyeHeight = 1.7f;

    cameraRight = glm::normalize(glm::cross(cameraViewDirection, cameraUp));
    clampToGround();
}

Camera::~Camera() {}


// ================= INTERNAL HELPERS =================

void Camera::updateCameraVectors()
{
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    cameraViewDirection = glm::normalize(direction);
    cameraRight = glm::normalize(glm::cross(cameraViewDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
    cameraUp = glm::normalize(glm::cross(cameraRight, cameraViewDirection));
}

void Camera::clampToGround()
{
    if (cameraPosition.y < groundHeight + eyeHeight)
        cameraPosition.y = groundHeight + eyeHeight;
}

// ================= MOVEMENT =================

// Forward/backward (GROUND ONLY)
void Camera::keyboardMoveFront(float speed)
{
    glm::vec3 forward = cameraViewDirection;
    forward.y = 0.0f;
    forward = glm::normalize(forward);

    cameraPosition += forward * speed;
    clampToGround();
}

void Camera::keyboardMoveBack(float speed)
{
    glm::vec3 forward = cameraViewDirection;
    forward.y = 0.0f;
    forward = glm::normalize(forward);

    cameraPosition -= forward * speed;
    clampToGround();
}

// Strafing (GROUND ONLY)
void Camera::keyboardMoveLeft(float speed)
{
    glm::vec3 right = cameraRight;
    right.y = 0.0f;
    right = glm::normalize(right);

    cameraPosition -= right * speed;
    clampToGround();
}

void Camera::keyboardMoveRight(float speed)
{
    glm::vec3 right = cameraRight;
    right.y = 0.0f;
    right = glm::normalize(right);

    cameraPosition += right * speed;
    clampToGround();
}

// Vertical movement (explicit)
void Camera::keyboardMoveUp(float speed)
{
    cameraPosition.y += speed;
}

void Camera::keyboardMoveDown(float speed)
{
    cameraPosition.y -= speed;
    clampToGround();
}

// ================= MOUSE =================

void Camera::processMouseMovement(float xoffset, float yoffset)
{
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)  pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    updateCameraVectors();
}

// ================= GETTERS =================

glm::mat4 Camera::getViewMatrix()
{
    return glm::lookAt(cameraPosition, cameraPosition + cameraViewDirection, cameraUp);
}

glm::vec3 Camera::getCameraPosition()
{
    return cameraPosition;
}

glm::vec3 Camera::getCameraViewDirection()
{
    return cameraViewDirection;
}

glm::vec3 Camera::getCameraUp()
{
    return cameraUp;
}
