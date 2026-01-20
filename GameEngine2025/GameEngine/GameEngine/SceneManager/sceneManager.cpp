#include "sceneManager.h"
#include <glew.h>
#include <iostream>

SceneManager::SceneManager()
    : starsMesh(nullptr),
    groundMesh(nullptr),
    currentSceneId(0),
    nearbyTrigger(-1),
    lightColor(1.0f, 1.0f, 1.0f),
    lightPos(0.0f, 500.0f, 0.0f)
{
}

SceneManager::~SceneManager()
{
}

void SceneManager::initializeResources()
{
    ResourceManager& rm = ResourceManager::getInstance();

    std::cout << "Loading resources..." << std::endl;

    // Load all textures
    rm.loadTexture("mars", "Resources/Textures/mars.bmp");
    rm.loadTexture("base_color", "Resources/Textures/Texture_1K/Base_BaseColor.bmp");
    rm.loadTexture("base_normal", "Resources/Textures/Texture_1K/Base_Normal.bmp");
    rm.loadTexture("gun", "Resources/Textures/SciFi_Gun_Full_Base/Paint_Base_Color.bmp");
    rm.loadTexture("cave_wall_diffuse", "Resources/Textures/CaveWalls2_Base_Diffuse.bmp");
    rm.loadTexture("asteroid_diffuse", "Resources/Textures/Asteroid_1_Diffuse_1K.bmp");
    rm.loadTexture("cave_wall4_diffuse", "Resources/Textures/CaveWalls4_Base_Diffuse.bmp");
    rm.loadTexture("alien_body", "Resources/Textures/body_Base_Color.bmp");
    rm.loadTexture("alien_eye", "Resources/Textures/eye_Base_Color.bmp");

    // Create procedural meshes
    starsMesh = rm.createStarField("stars", 500, 2000.0f);
    groundMesh = rm.createGround("ground", 200.0f, "mars");

    // Load spaceship mesh with textures
    Mesh* shipMesh = rm.loadMesh("spaceship", "Resources/Models/Imperial_Steniel_obj.obj");
    std::vector<Texture> shipTextures;
    shipTextures.push_back({ rm.getTexture("base_color"), "texture_diffuse" });
    shipTextures.push_back({ rm.getTexture("base_normal"), "texture_normal" });
    shipMesh->setTextures(shipTextures);

    // Load cave wall meshes
    rm.loadMesh("cave_wall_a", "Resources/Models/CaveWalls2_A.obj");
    rm.getMesh("cave_wall_a")->setTextures({ { rm.getTexture("cave_wall_diffuse"), "texture_diffuse" } });

    rm.loadMesh("cave_wall_b", "Resources/Models/CaveWalls2_B.obj");
    rm.getMesh("cave_wall_b")->setTextures({ { rm.getTexture("cave_wall_diffuse"), "texture_diffuse" } });

    rm.loadMesh("cave_wall_c", "Resources/Models/CaveWalls2_C.obj");
    rm.getMesh("cave_wall_c")->setTextures({ { rm.getTexture("cave_wall_diffuse"), "texture_diffuse" } });

    rm.loadMesh("cave_wall_set", "Resources/Models/CaveWalls2_Set.obj");
    rm.getMesh("cave_wall_set")->setTextures({ { rm.getTexture("cave_wall_diffuse"), "texture_diffuse" } });

    // Load asteroid
    rm.loadMesh("asteroid", "Resources/Models/Asteroid_1.obj");
    rm.getMesh("asteroid")->setTextures({ { rm.getTexture("asteroid_diffuse"), "texture_diffuse" } });

    // Load cave wall 4 set
    rm.loadMesh("cave_wall4_set", "Resources/Models/CaveWalls4_Set.obj");
    rm.getMesh("cave_wall4_set")->setTextures({ { rm.getTexture("cave_wall4_diffuse"), "texture_diffuse" } });

    // Load rock models
    rm.loadMesh("rock04_a", "Resources/Models/Rock04_A.obj");
    rm.getMesh("rock04_a")->setTextures({ { rm.getTexture("cave_wall_diffuse"), "texture_diffuse" } });

    rm.loadMesh("rock04_b", "Resources/Models/Rock04_B.obj");
    rm.getMesh("rock04_b")->setTextures({ { rm.getTexture("cave_wall_diffuse"), "texture_diffuse" } });

    rm.loadMesh("rock04_c", "Resources/Models/Rock04_C.obj");
    rm.getMesh("rock04_c")->setTextures({ { rm.getTexture("cave_wall_diffuse"), "texture_diffuse" } });

    rm.loadMesh("rock04_d", "Resources/Models/Rock04_D.obj");
    rm.getMesh("rock04_d")->setTextures({ { rm.getTexture("cave_wall4_diffuse"), "texture_diffuse" } });

    rm.loadMesh("rock04_e", "Resources/Models/Rock04_E.obj");
    rm.getMesh("rock04_e")->setTextures({ { rm.getTexture("cave_wall4_diffuse"), "texture_diffuse" } });

    rm.loadMesh("rock04_set", "Resources/Models/Rock04_Set.obj");
    rm.getMesh("rock04_set")->setTextures({ { rm.getTexture("cave_wall_diffuse"), "texture_diffuse" } });

    // Load alien
    rm.loadMesh("alien", "Resources/Models/body.obj");
    rm.getMesh("alien")->setTextures({ { rm.getTexture("alien_body"), "texture_diffuse" } });

    std::cout << "Resources loaded successfully!" << std::endl;
}

void SceneManager::clearScene()
{
    std::cout << "Clearing current scene..." << std::endl;

    spaceships.clear();
    aliens.clear();
    caveWalls.clear();
    rocks.clear();
    asteroids.clear();
    portalMarkers.clear();
    triggerZones.clear();
    nearbyTrigger = -1;

    currentSceneId = 0;
}

void SceneManager::loadScene(int sceneId)
{
    clearScene();

    std::cout << "Loading scene " << sceneId << "..." << std::endl;

    currentSceneId = sceneId;

    switch (sceneId)
    {
    case 1:
        createScene1();
        break;
    case 2:
        createScene2();
        break;
    default:
        std::cout << "Warning: Unknown scene ID " << sceneId << ", loading scene 1" << std::endl;
        createScene1();
        currentSceneId = 1;
        break;
    }

    std::cout << "Scene " << currentSceneId << " loaded successfully!" << std::endl;
}

// ==================== HELPER METHODS FOR ADDING OBJECTS ====================

void SceneManager::addSpaceship(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale)
{
    ResourceManager& rm = ResourceManager::getInstance();
    Mesh* shipMesh = rm.getMesh("spaceship");

    spaceships.push_back(std::make_unique<GameObject>(shipMesh, pos, rot, scale));
}

void SceneManager::addAlien(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale)
{
    ResourceManager& rm = ResourceManager::getInstance();
    Mesh* alienMesh = rm.getMesh("alien");

    aliens.push_back(std::make_unique<GameObject>(alienMesh, pos, rot, scale));
}

void SceneManager::addCaveWall(const std::string& meshName, const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale)
{
    ResourceManager& rm = ResourceManager::getInstance();
    Mesh* wallMesh = rm.getMesh(meshName);

    caveWalls.push_back(std::make_unique<GameObject>(wallMesh, pos, rot, scale));
}

void SceneManager::addRock(const std::string& meshName, const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale)
{
    ResourceManager& rm = ResourceManager::getInstance();
    Mesh* rockMesh = rm.getMesh(meshName);

    rocks.push_back(std::make_unique<GameObject>(rockMesh, pos, rot, scale));
}

void SceneManager::addAsteroid(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale)
{
    ResourceManager& rm = ResourceManager::getInstance();
    Mesh* asteroidMesh = rm.getMesh("asteroid");

    asteroids.push_back(std::make_unique<GameObject>(asteroidMesh, pos, rot, scale));
}

void SceneManager::addTriggerZone(const glm::vec3& pos, float radius, int targetScene, const std::string& message)
{
    TriggerZone trigger;
    trigger.position = pos;
    trigger.radius = radius;
    trigger.targetScene = targetScene;
    trigger.message = message;
    triggerZones.push_back(trigger);
}

void SceneManager::addPortalMarker(const glm::vec3& pos)
{
    ResourceManager& rm = ResourceManager::getInstance();
    Mesh* asteroidMesh = rm.getMesh("asteroid");

    // Create a glowing asteroid as visual marker
    portalMarkers.push_back(std::make_unique<GameObject>(asteroidMesh, pos,
        glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(3.0f)));
}

void SceneManager::checkProximityTriggers(const glm::vec3& playerPos)
{
    nearbyTrigger = -1;

    for (int i = 0; i < triggerZones.size(); i++)
    {
        float distance = glm::distance(playerPos, triggerZones[i].position);
        if (distance < triggerZones[i].radius)
        {
            nearbyTrigger = i;
            return;
        }
    }
}

std::string SceneManager::getTriggerMessage() const
{
    if (nearbyTrigger >= 0 && nearbyTrigger < triggerZones.size())
        return triggerZones[nearbyTrigger].message;
    return "";
}

// ==================== SCENE CREATION METHODS ====================

void SceneManager::createScene1()
{
    std::cout << "Creating Scene 1: Cave Entrance..." << std::endl;

    // Add spaceship
    addSpaceship(glm::vec3(-20.0f, -2.5f, -50.0f), glm::vec3(0.0f, 180.0f, 0.0f), glm::vec3(2.5f));

    // Add alien
    addAlien(glm::vec3(15.0f, -8.0f, -50.0f), glm::vec3(0.0f, 180.0f, 0.0f), glm::vec3(1.5f));

    // Add cave walls
    addCaveWall("cave_wall_set", glm::vec3(-80.0f, -9.0f, -120.0f), glm::vec3(0.0f, 45.0f, 0.0f), glm::vec3(3.0f));
    addCaveWall("cave_wall_a", glm::vec3(40.0f, -8.0f, -260.0f), glm::vec3(0.0f, -30.0f, 0.0f), glm::vec3(2.5f));
    addCaveWall("cave_wall_a", glm::vec3(-30.0f, -8.5f, 400.0f), glm::vec3(0.0f, 60.0f, 0.0f), glm::vec3(2.0f));
    addCaveWall("cave_wall_b", glm::vec3(-70.0f, -7.0f, -350.0f), glm::vec3(0.0f, 90.0f, 0.0f), glm::vec3(4.0f, 3.0f, 4.0f));
    addCaveWall("cave_wall_c", glm::vec3(-100.0f, -6.0f, -480.0f), glm::vec3(0.0f, 120.0f, 0.0f), glm::vec3(3.5f));
    addCaveWall("cave_wall4_set", glm::vec3(100.0f, 10.0f, 350.0f), glm::vec3(0.0f, 90.0f, 0.0f), glm::vec3(2.0f));

    // Add rocks
    addRock("rock04_a", glm::vec3(-150.0f, -8.0f, 200.0f), glm::vec3(0.0f, 45.0f, 0.0f), glm::vec3(3.0f));
    addRock("rock04_b", glm::vec3(-180.0f, -7.0f, -50.0f), glm::vec3(0.0f, -30.0f, 0.0f), glm::vec3(2.5f));
    addRock("rock04_c", glm::vec3(150.0f, -8.0f, -80.0f), glm::vec3(0.0f, 135.0f, 0.0f), glm::vec3(3.0f));
    addRock("rock04_d", glm::vec3(170.0f, -7.5f, 20.0f), glm::vec3(0.0f, 200.0f, 0.0f), glm::vec3(2.8f));
    addRock("rock04_e", glm::vec3(-160.0f, -8.5f, 80.0f), glm::vec3(0.0f, 75.0f, 0.0f), glm::vec3(2.5f));
    addRock("rock04_set", glm::vec3(140.0f, -8.0f, 50.0f), glm::vec3(0.0f, 160.0f, 0.0f), glm::vec3(2.0f));
    addRock("rock04_c", glm::vec3(-140.0f, -8.0f, 30.0f), glm::vec3(0.0f, -45.0f, 0.0f), glm::vec3(2.8f));
    addRock("rock04_d", glm::vec3(-175.0f, -7.8f, 150.0f), glm::vec3(0.0f, 60.0f, 0.0f), glm::vec3(3.2f));
    addRock("rock04_a", glm::vec3(-130.0f, -8.2f, -30.0f), glm::vec3(0.0f, 110.0f, 0.0f), glm::vec3(2.6f));
    addRock("rock04_set", glm::vec3(-155.0f, -7.5f, -120.0f), glm::vec3(0.0f, -80.0f, 0.0f), glm::vec3(2.2f));
    addRock("rock04_b", glm::vec3(-165.0f, -8.3f, 250.0f), glm::vec3(0.0f, 25.0f, 0.0f), glm::vec3(2.9f));

    // Add asteroid
    addAsteroid(glm::vec3(15.0f, 40.0f, -50.0f), glm::vec3(35.0f * 1.0f, 35.0f * 0.5f, 35.0f * 0.3f), glm::vec3(7.0f));

    // Add trigger zone at cave_wall_a position to go to scene 2
    glm::vec3 triggerPos = glm::vec3(-30.0f, -8.5f, 400.0f);
    addTriggerZone(triggerPos, 25.0f, 2, "Press 'N' to enter the Deep Cave");

    // Add visual portal marker (glowing asteroid)
    addPortalMarker(glm::vec3(-30.0f, -5.0f, 400.0f)); // Slightly above ground
}

void SceneManager::createScene2()
{
    std::cout << "Creating Scene 2" << std::endl;

    addCaveWall("cave_wall_a", glm::vec3(-30.0f, -8.5f, 400.0f), glm::vec3(0.0f, 60.0f, 0.0f), glm::vec3(2.0f));
    
}

void SceneManager::setupLighting(Shader& shader, const glm::vec3& cameraPos)
{
    glUniform3fv(glGetUniformLocation(shader.getId(), "lightColor"), 1, &lightColor[0]);
    glUniform3fv(glGetUniformLocation(shader.getId(), "lightPos"), 1, &lightPos[0]);
    glUniform3fv(glGetUniformLocation(shader.getId(), "viewPos"), 1, &cameraPos[0]);
}

void SceneManager::setEnhancedLighting(Shader& shader)
{
    glUniform1f(glGetUniformLocation(shader.getId(), "ambientStrength"), 0.7f);
    glUniform1f(glGetUniformLocation(shader.getId(), "specularStrength"), 1.5f);
    glm::vec3 brightLightColor = glm::vec3(2.2f, 2.2f, 2.2f);
    glUniform3fv(glGetUniformLocation(shader.getId(), "lightColor"), 1, &brightLightColor[0]);
}

void SceneManager::setDimLighting(Shader& shader)
{
    glUniform1f(glGetUniformLocation(shader.getId(), "ambientStrength"), 0.1f);
    glUniform1f(glGetUniformLocation(shader.getId(), "specularStrength"), 0.3f);
}

void SceneManager::setNormalLighting(Shader& shader)
{
    glUniform1f(glGetUniformLocation(shader.getId(), "ambientStrength"), 0.2f);
    glUniform1f(glGetUniformLocation(shader.getId(), "specularStrength"), 0.5f);
    glUniform3fv(glGetUniformLocation(shader.getId(), "lightColor"), 1, &lightColor[0]);
}

void SceneManager::renderStars(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, Shader& sunShader)
{
    sunShader.use();
    glm::mat4 MVP = projectionMatrix * viewMatrix;
    glUniformMatrix4fv(glGetUniformLocation(sunShader.getId(), "MVP"), 1, GL_FALSE, &MVP[0][0]);

    glPointSize(2.0f);
    if (starsMesh)
    {
        starsMesh->drawPoints(sunShader);
    }
}

void SceneManager::renderGround(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix,
    const glm::vec3& cameraPos, Shader& shader)
{
    shader.use();
    GLuint MatrixID = glGetUniformLocation(shader.getId(), "MVP");
    GLuint ModelID = glGetUniformLocation(shader.getId(), "model");

    setupLighting(shader, cameraPos);
    setNormalLighting(shader);

    float camX = cameraPos.x;
    float camZ = cameraPos.z;
    float tileSize = 400.0f;

    int tileX = (int)floor(camX / tileSize);
    int tileZ = (int)floor(camZ / tileSize);

    for (int x = -1; x <= 1; x++)
    {
        for (int z = -1; z <= 1; z++)
        {
            glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0f),
                glm::vec3((tileX + x) * tileSize, -10.0f, (tileZ + z) * tileSize));

            glm::mat4 MVP = projectionMatrix * viewMatrix * ModelMatrix;
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
            glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);

            if (groundMesh)
            {
                groundMesh->draw(shader);
            }
        }
    }
}

void SceneManager::render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix,
    const glm::vec3& cameraPos, Shader& shader)
{
    shader.use();
    GLuint MatrixID = glGetUniformLocation(shader.getId(), "MVP");
    GLuint ModelID = glGetUniformLocation(shader.getId(), "model");

    setupLighting(shader, cameraPos);

    // Render spaceships
    setEnhancedLighting(shader);
    for (auto& ship : spaceships)
    {
        glm::mat4 modelMatrix = ship->getModelMatrix();
        glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &modelMatrix[0][0]);
        ship->draw(shader);
    }

    // Render cave walls
    setNormalLighting(shader);
    for (auto& wall : caveWalls)
    {
        glm::mat4 modelMatrix = wall->getModelMatrix();
        glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &modelMatrix[0][0]);
        wall->draw(shader);
    }

    // Render rocks
    for (auto& rock : rocks)
    {
        glm::mat4 modelMatrix = rock->getModelMatrix();
        glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &modelMatrix[0][0]);
        rock->draw(shader);
    }

    // Render aliens
    setEnhancedLighting(shader);
    for (auto& alien : aliens)
    {
        glm::mat4 modelMatrix = alien->getModelMatrix();
        glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &modelMatrix[0][0]);
        alien->draw(shader);
    }

    // Render asteroids
    for (auto& asteroid : asteroids)
    {
        glm::mat4 modelMatrix = asteroid->getModelMatrix();
        glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &modelMatrix[0][0]);
        asteroid->draw(shader);
    }

    // Render portal markers
    for (auto& portal : portalMarkers)
    {
        glm::mat4 modelMatrix = portal->getModelMatrix();
        glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &modelMatrix[0][0]);
        portal->draw(shader);
    }

    setNormalLighting(shader);
}