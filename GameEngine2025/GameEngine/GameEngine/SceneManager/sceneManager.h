#pragma once
#include <vector>
#include <memory>
#include <string>
#include "../GameObject/gameObject.h"
#include "../Shaders/shader.h"
#include "../ResourceManager/resourceManager.h"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

struct TriggerZone {
    glm::vec3 position;
    float radius;
    int targetScene;
    std::string message;
};

class SceneManager
{
public:
    SceneManager();
    ~SceneManager();

    // Initialize resources (called once at startup)
    void initializeResources();

    // Scene management
    void loadScene(int sceneId);
    void clearScene();
    int getCurrentScene() const { return currentSceneId; }

    // Trigger system
    void checkProximityTriggers(const glm::vec3& playerPos);
    int getNearbyTrigger() const { return nearbyTrigger; }
    std::string getTriggerMessage() const;
    const std::vector<TriggerZone>& getTriggerZones() const { return triggerZones; }

    // Rendering
    void render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix,
        const glm::vec3& cameraPos, Shader& shader);

    void renderStars(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, Shader& sunShader);
    void renderGround(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix,
        const glm::vec3& cameraPos, Shader& shader);

private:
    // Scene objects
    std::vector<std::unique_ptr<GameObject>> spaceships;
    std::vector<std::unique_ptr<GameObject>> aliens;
    std::vector<std::unique_ptr<GameObject>> caveWalls;
    std::vector<std::unique_ptr<GameObject>> rocks;
    std::vector<std::unique_ptr<GameObject>> asteroids;
    std::vector<std::unique_ptr<GameObject>> portalMarkers; // Visual indicators for triggers

    Mesh* starsMesh;
    Mesh* groundMesh;

    int currentSceneId;

    // Trigger zones
    std::vector<TriggerZone> triggerZones;
    int nearbyTrigger; // -1 if none, else index of trigger

    // Lighting parameters
    glm::vec3 lightColor;
    glm::vec3 lightPos;

    // Lighting setup
    void setupLighting(Shader& shader, const glm::vec3& cameraPos);
    void setEnhancedLighting(Shader& shader);
    void setDimLighting(Shader& shader);
    void setNormalLighting(Shader& shader);

    // Scene creation methods
    void createScene1();  // Cave entrance with spaceship
    void createScene2();  // Deep cave with more aliens
    void createScene3();  // Underground chamber

    // Helper methods for creating object groups (used by scenes)
    void addSpaceship(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale);
    void addAlien(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale);
    void addCaveWall(const std::string& meshName, const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale);
    void addRock(const std::string& meshName, const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale);
    void addAsteroid(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale);

    // Trigger system helpers
    void addTriggerZone(const glm::vec3& pos, float radius, int targetScene, const std::string& message);
    void addPortalMarker(const glm::vec3& pos); // Visual indicator
};