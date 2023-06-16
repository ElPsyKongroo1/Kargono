#pragma once
#include "../../../Library/Includes.h"
#include "../../../Library/Rendering/Mesh/Meshes.h"


class ResourceManager 
{
public:
	std::vector<Texture*> applicationTextures;
	std::vector<GLShader*> applicationShaders;
	std::vector<GLMesh*> applicationMeshes;
public:
	ResourceManager() : applicationTextures{std::vector<Texture*>()}, 
					    applicationShaders{ std::vector<GLShader*>() },
						applicationMeshes{ std::vector<GLMesh*>() }
	{
	
	}
	~ResourceManager() 
	{
		for (Texture* texture : applicationTextures)
		{
			delete texture;
			texture = nullptr;
		}
		for (GLShader* shader : applicationShaders)
		{
			delete shader;
			shader = nullptr;
		}
		for (GLMesh* mesh : applicationMeshes)
		{
			if (!mesh) { continue; }
			delete mesh;
			mesh = nullptr;
		}
		applicationTextures.clear();
		applicationShaders.clear();
		applicationMeshes.clear();
	}
public:
	void initializeResources();

};