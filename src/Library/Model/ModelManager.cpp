#include "Model.h"


void ModelManager::CreateModels()
{
	CreateBackpack(simpleBackpack);
	CreateHuman(human);
	CreateHouse(house);
	CreateHouse2(house2);

}
void ModelManager::DestroyModels()
{
	DestroyModel(simpleBackpack);
	DestroyModel(human);
	DestroyModel(house);
	DestroyModel(house2);

}

void ModelManager::CreateBackpack(Model& model)
{
	std::string newString = "Resources/Models/backpack/backpack.obj";
	char* character = &newString.at(0);
	model = Model(character);
}

void ModelManager::CreateHuman(Model& model)
{
	std::string newString = "Resources/Models/SimpleHuman.obj";
	char* character = &newString.at(0);
	model = Model(character);
}

void ModelManager::CreateHouse(Model& model)
{
	std::string newString = "Resources/Models/Buildings/Residential Buildings 001.obj";
	char* character = &newString.at(0);
	model = Model(character);
}

void ModelManager::CreateHouse2(Model& model)
{
	std::string newString = "Resources/Models/Buildings/Residential Buildings 002.obj";
	char* character = &newString.at(0);
	model = Model(character);
}


void ModelManager::DestroyModel(Model& model)
{
	for (GLMesh mesh : model.meshes)
	{
		glDeleteBuffers(2, mesh.vbos);
		mesh.vertices.clear();
		mesh.indices.clear();
		for (Texture texture : mesh.textures)
		{
			glDeleteTextures(1, &texture.id);
		}
	}
	model.meshes.clear();
	model.textures_loaded.clear();
}