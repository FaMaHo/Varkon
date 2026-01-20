#pragma once
#include <map>
#include <string>
#include <memory>
#include "../Model Loading/mesh.h"
#include "../Model Loading/texture.h"
#include "../Model Loading/meshLoaderObj.h"

class ResourceManager
{
public:
    static ResourceManager& getInstance();

    // Load and cache textures
    GLuint loadTexture(const std::string& name, const std::string& path);
    GLuint getTexture(const std::string& name);

    // Load and cache meshes
    Mesh* loadMesh(const std::string& name, const std::string& path);
    Mesh* loadMesh(const std::string& name, const std::string& path, const std::vector<std::string>& textureNames);
    Mesh* getMesh(const std::string& name);

    // Procedural meshes
    Mesh* createStarField(const std::string& name, int numStars, float spaceSize);
    Mesh* createGround(const std::string& name, float size, const std::string& textureName);

    void cleanup();

private:
    ResourceManager() {}
    ~ResourceManager();
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    std::map<std::string, GLuint> textures;
    std::map<std::string, std::unique_ptr<Mesh>> meshes;
    MeshLoaderObj meshLoader;

    Mesh* createStarsInternal(int numStars, float spaceSize);
    Mesh* createGroundInternal(float size, GLuint textureId);
};