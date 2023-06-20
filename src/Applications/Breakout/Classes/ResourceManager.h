#pragma once
#include "../../../Library/Includes.h"
#include "../../../Library/Rendering/Mesh/Meshes.h"
#include "../../../Library/Application/Input/Input.h"


class ResourceManager 
{
public:
	std::vector<Texture*> applicationTextures;
	std::vector<GLShader*> applicationShaders;
	std::vector<GLMesh*> applicationMeshes;
	std::vector<GLInput*> applicationInputs;
	float dimensionConversionFactor = 0.5;
public:
	ResourceManager() : applicationTextures{std::vector<Texture*>()}, 
					    applicationShaders{ std::vector<GLShader*>() },
						applicationMeshes{ std::vector<GLMesh*>() },
						applicationInputs{std::vector<GLInput*>()}
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
		for (GLInput* input : applicationInputs)
		{
			if (!input) { continue; }
			delete input;
			input = nullptr;
		}
		applicationTextures.clear();
		applicationShaders.clear();
		applicationMeshes.clear();
		applicationInputs.clear();
	}
public:
	void initializeResources();
private:
	void initializeInput();

};