#include "GameLevel.h"
#include "../../../Library/Library.h"
#include "ResourceManager.h"
void GameLevel::Load(const char* file) 
{
	int tileCode;
	std::string line;
	std::ifstream fstream(file);
	int i{ 0 };
	while (std::getline(fstream, line) && (i < levelHeight))
	{
		int j{ 0 };
		std::istringstream sstream(line);
		std::vector<int> row;
		while (sstream >> tileCode && (j < levelWidth)) 
		{ 
			initMap.push_back(tileCode);
			j++;
		}
		i++;
	}
	init(800, 600); //FIXME
}
void GameLevel::init(int cameraViewWidth, int cameraViewHeight) 
{
	float xLocation;
	float yLocation;
	float xSpace = 2.0f;
	float ySpace = 3.5f;
	float unitWidth = 30.0f;
	float unitHeight = 30.0f;
	float xInitial = -((cameraViewWidth / 2) - (unitWidth / 2));
	float yInitial = (cameraViewHeight / 2) - (unitHeight / 2);
	Orientation orientation2;
	ShapeRenderer* renderer2;
	for (int i{ 0 }; i < levelHeight; i++)
	{
		for (int j{ 0 }; j < levelWidth; j++)
		{
			if (initMap.at((i * levelWidth) + j) == 0) 
			{
				currentMap.push_back(nullptr);
				continue;
			}
			xLocation = xInitial + (j * (unitWidth + xSpace));
			yLocation = yInitial - (i * (unitHeight + ySpace));
			orientation2 = { glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
						  glm::vec3(xLocation, yLocation, -0.5f),
						  glm::vec3(30.0f, 30.0f, 0.5f) };
			renderer2 = { new ShapeRenderer(orientation2,
				Resources::currentGame->resourceManager->applicationMeshes.at(initMap.at((i * levelWidth) + j) - 1),
				Resources::currentApplication->renderer->defaultShader) };
			currentMap.push_back(new GameObject(orientation2, renderer2));
		}
	}
}
