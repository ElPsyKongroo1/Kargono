#pragma once
#include "../../../Library/Includes.h"
#include "../../../Library/Rendering/Mesh/Meshes.h"
#include "../../../Library/Application/Input/Input.h"


class ResourceManager 
{
public:
	std::vector<Texture*> localTextures;
	std::vector<GLShader*> localShaders;
	std::vector<GLMesh*> localMeshes;
	std::vector<GLInput*> localInputs;
	float dimensionConversionFactor = 0.5;
public:
	ResourceManager() : localTextures{std::vector<Texture*>()}, 
					    localShaders{ std::vector<GLShader*>() },
						localMeshes{ std::vector<GLMesh*>() },
						localInputs{std::vector<GLInput*>()}
	{
	
	}
	~ResourceManager() 
	{
		for (Texture* texture : localTextures)
		{
			delete texture;
			texture = nullptr;
		}
		for (GLShader* shader : localShaders)
		{
			delete shader;
			shader = nullptr;
		}
		for (GLMesh* mesh : localMeshes)
		{
			if (!mesh) { continue; }
			delete mesh;
			mesh = nullptr;
		}
		for (GLInput* input : localInputs)
		{
			if (!input) { continue; }
			delete input;
			input = nullptr;
		}
		localTextures.clear();
		localShaders.clear();
		localMeshes.clear();
		localInputs.clear();
	}
public:
	void initializeResources();
private:
	void initializeInput();

};