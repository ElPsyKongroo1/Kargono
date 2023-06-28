#include "Model.h"


void ModelManager::CreateModels()
{
	CreateBackpack();
	CreateHuman();
	CreateHouse();
	CreateHouse2();
}
void ModelManager::DestroyModels()
{
	delete simpleBackpack;
	delete human;
	delete house;
	delete house2;
}

void ModelManager::CreateBackpack()
{
	std::string newString = "Resources/Default-Resources/Models/backpack/backpack.obj";
	char* character = &newString.at(0);
	simpleBackpack = new Model(character);
}

void ModelManager::CreateHuman()
{
	std::string newString = "Resources/Default-Resources/Models/SimpleHuman.obj";
	char* character = &newString.at(0);
	human = new Model(character);
}

void ModelManager::CreateHouse()
{
	std::string newString = "Resources/Default-Resources/Models/Buildings/Residential Buildings 001.obj";
	char* character = &newString.at(0);
	house = new Model(character);
}

void ModelManager::CreateHouse2()
{
	std::string newString = "Resources/Default-Resources/Models/Buildings/Residential Buildings 002.obj";
	char* character = &newString.at(0);
	house2 = new Model(character);
}