#include "ResourceManager.h"

void ResourceManager::initializeResources() 
{
	Texture* purpleBlock = new Texture("Resources/Breakout/Textures/Original/PurpleBlock.png", "texture_diffuse");
	Texture* blueBlock = new Texture("Resources/Breakout/Textures/Original/BlueBlock.png", "texture_diffuse");
	Texture* specBlock = new Texture("Resources/Breakout/Textures/SpecularMap/SpecularBlock.png", "texture_specular");
	Texture* basePaddle = new Texture("Resources/Breakout/Textures/Original/BasePaddle.png", "texture_diffuse");
	Texture* specPaddle = new Texture("Resources/Breakout/Textures/SpecularMap/specPaddle.png", "texture_specular");
    Texture* baseBall = new Texture("Resources/Breakout/Textures/Original/BaseBall.png", "texture_diffuse");
    Texture* specBall = new Texture("Resources/Breakout/Textures/SpecularMap/specBall.png", "texture_specular");


    this->applicationTextures.push_back(purpleBlock);
    this->applicationTextures.push_back(specBlock);

    glm::vec3 dimensions = glm::vec3(0.5f, 0.5f, 0.0f);

    std::vector<unsigned int> indices {std::vector<unsigned int>()};
    std::vector<Texture*> textures {std::vector<Texture*>()};
    textures.push_back(purpleBlock);
    textures.push_back(specBlock);
    std::vector<Vertex> vertices
    {
            Vertex(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
            Vertex(glm::vec3(0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
            Vertex(glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)),
            Vertex(glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)),
            Vertex(glm::vec3(-0.5f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)),
            Vertex(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f))
    };
    GLMesh* purpleMesh{ new GLMesh(vertices, indices, textures, GLMesh::DRAWARRAYS, dimensions) };
    applicationMeshes.push_back(purpleMesh);

    textures.clear();
    textures.push_back(blueBlock);
    textures.push_back(specBlock);
    GLMesh* blueMesh{ new GLMesh(vertices, indices, textures, GLMesh::DRAWARRAYS, dimensions) };
    applicationMeshes.push_back(blueMesh);


    textures.clear();
    textures.push_back(baseBall);
    textures.push_back(specBall);

    GLMesh* ballMesh{ new GLMesh(vertices, indices, textures, GLMesh::DRAWARRAYS, dimensions) };
    applicationMeshes.push_back(ballMesh);


    vertices = 
    {
            Vertex(glm::vec3(-0.5f, -0.125f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
            Vertex(glm::vec3(0.5f, -0.125f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
            Vertex(glm::vec3(0.5f, 0.125f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)),
            Vertex(glm::vec3(0.5f, 0.125f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)),
            Vertex(glm::vec3(-0.5f, 0.125f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)),
            Vertex(glm::vec3(-0.5f, -0.125f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f))
    };
    textures.clear();
    textures.push_back(basePaddle);
    textures.push_back(specPaddle);
    dimensions = glm::vec3(0.5f, 0.125f, 0.0f);

    GLMesh* paddleMesh{ new GLMesh(vertices, indices, textures, GLMesh::DRAWARRAYS, dimensions) };
    applicationMeshes.push_back(paddleMesh);
}