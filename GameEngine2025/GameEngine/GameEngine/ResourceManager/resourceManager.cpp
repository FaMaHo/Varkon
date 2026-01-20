#include "resourceManager.h"
#include <iostream>

ResourceManager& ResourceManager::getInstance()
{
    static ResourceManager instance;
    return instance;
}

ResourceManager::~ResourceManager()
{
    cleanup();
}

GLuint ResourceManager::loadTexture(const std::string& name, const std::string& path)
{
    // Check if already loaded
    auto it = textures.find(name);
    if (it != textures.end())
    {
        return it->second;
    }

    // Load new texture
    GLuint textureId = loadBMP(path.c_str());
    textures[name] = textureId;
    std::cout << "Loaded texture: " << name << " from " << path << std::endl;
    return textureId;
}

GLuint ResourceManager::getTexture(const std::string& name)
{
    auto it = textures.find(name);
    if (it != textures.end())
    {
        return it->second;
    }
    std::cout << "Warning: Texture '" << name << "' not found!" << std::endl;
    return 0;
}

Mesh* ResourceManager::loadMesh(const std::string& name, const std::string& path)
{
    // Check if already loaded
    auto it = meshes.find(name);
    if (it != meshes.end())
    {
        return it->second.get();
    }

    // Load new mesh
    Mesh loadedMesh = meshLoader.loadObj(path);
    meshes[name] = std::make_unique<Mesh>(loadedMesh);
    std::cout << "Loaded mesh: " << name << " from " << path << std::endl;
    return meshes[name].get();
}

Mesh* ResourceManager::loadMesh(const std::string& name, const std::string& path,
    const std::vector<std::string>& textureNames)
{
    // Check if already loaded
    auto it = meshes.find(name);
    if (it != meshes.end())
    {
        return it->second.get();
    }

    // Build texture vector
    std::vector<Texture> textureList;
    for (const auto& texName : textureNames)
    {
        GLuint texId = getTexture(texName);
        if (texId != 0)
        {
            Texture tex;
            tex.id = texId;
            tex.type = "texture_diffuse"; // Default, can be customized
            textureList.push_back(tex);
        }
    }

    // Load mesh with textures
    Mesh loadedMesh = meshLoader.loadObj(path, textureList);
    meshes[name] = std::make_unique<Mesh>(loadedMesh);
    std::cout << "Loaded mesh with textures: " << name << " from " << path << std::endl;
    return meshes[name].get();
}

Mesh* ResourceManager::getMesh(const std::string& name)
{
    auto it = meshes.find(name);
    if (it != meshes.end())
    {
        return it->second.get();
    }
    std::cout << "Warning: Mesh '" << name << "' not found!" << std::endl;
    return nullptr;
}

Mesh* ResourceManager::createStarField(const std::string& name, int numStars, float spaceSize)
{
    auto it = meshes.find(name);
    if (it != meshes.end())
    {
        return it->second.get();
    }

    Mesh* stars = createStarsInternal(numStars, spaceSize);
    meshes[name] = std::unique_ptr<Mesh>(stars);
    return meshes[name].get();
}

Mesh* ResourceManager::createGround(const std::string& name, float size, const std::string& textureName)
{
    auto it = meshes.find(name);
    if (it != meshes.end())
    {
        return it->second.get();
    }

    GLuint texId = getTexture(textureName);
    Mesh* ground = createGroundInternal(size, texId);
    meshes[name] = std::unique_ptr<Mesh>(ground);
    return meshes[name].get();
}

Mesh* ResourceManager::createStarsInternal(int numStars, float spaceSize)
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
    return new Mesh(vertices, indices);
}

Mesh* ResourceManager::createGroundInternal(float size, GLuint textureId)
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

    return new Mesh(vertices, indices, textures);
}

void ResourceManager::cleanup()
{
    meshes.clear();
    textures.clear();
}