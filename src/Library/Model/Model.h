#pragma once
#include "../Includes.h"
#include "../Mesh/Meshes.h"
#include "../Textures/Textures.h"
#include "../Shaders/Shaders.h"


class Model
{
public:
	std::vector<Texture> textures_loaded;
public:
	Model() {};
	Model(char* path)
	{
		loadModel(path);
	}
	void Draw(void* object);
public:
		std::vector<GLMesh> meshes;
		std::string directory;
private:
		void loadModel(std::string path);
		void processNode(aiNode* Node, const aiScene* scene);
		GLMesh processMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Texture> loadMaterialTextures(aiMaterial* mat,
			aiTextureType type,
			std::string typeName);
};

class ModelManager
{
public:
	Model simpleBackpack;
	Model human;
	Model house;
	Model house2;

public:
	ModelManager() {};

public:
	void CreateModels();
	void DestroyModels();

private:
	void CreateBackpack(Model& model);
	void CreateHuman(Model& model);
	void CreateHouse(Model& model);
	void CreateHouse2(Model& model);
	void DestroyModel(Model& model);

};