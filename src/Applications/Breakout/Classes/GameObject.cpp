#include "GameObject.h"
#include "../../../Library/Library.h"
//void GameObject::setBaseDimensions(glm::vec3 baseDimensions)
//{
//		objectBaseDimensions = baseDimensions;
//		objectCurrentDimensions = glm::vec3(
//			((baseDimensions.x / Resources::currentGame->resourceManager->dimensionConversionFactor) * orientation.scale.x),
//			((baseDimensions.y / Resources::currentGame->resourceManager->dimensionConversionFactor) * orientation.scale.y),
//			((baseDimensions.z / Resources::currentGame->resourceManager->dimensionConversionFactor) * orientation.scale.z));
//	}

void GameBrick::remove() 
{

}

void GameBall::Move()
{
	orientation.translation = glm::vec3(orientation.translation.x + (direction.x * objectSpeed * Resources::deltaTime),
		orientation.translation.y + (direction.y * objectSpeed * Resources::deltaTime),
		0.0f);
	float maxBorderX = Resources::currentGame->renderer->currentCamera->frustrumDimensions.widthDimension.y - (static_cast<ShapeRenderer*>(renderer)->mesh->dimensions.x * orientation.scale.x);
	float minBorderX = -(Resources::currentGame->renderer->currentCamera->frustrumDimensions.widthDimension.y - (static_cast<ShapeRenderer*>(renderer)->mesh->dimensions.x * orientation.scale.x));
	float minBorderY = -(Resources::currentGame->renderer->currentCamera->frustrumDimensions.heightDimension.y - (static_cast<ShapeRenderer*>(renderer)->mesh->dimensions.y * orientation.scale.y));
	float maxBorderY = Resources::currentGame->renderer->currentCamera->frustrumDimensions.heightDimension.y - (static_cast<ShapeRenderer*>(renderer)->mesh->dimensions.y * orientation.scale.y);
	if (orientation.translation.x >= maxBorderX)
	{
		orientation.translation.x = maxBorderX;
		direction.x = -direction.x;
	}
	if (orientation.translation.x <= minBorderX)
	{
		orientation.translation.x = minBorderX;
		direction.x = -direction.x;
	}
	if (orientation.translation.y <= minBorderY)
	{
		orientation.translation.y = minBorderY;
		direction.y = -direction.y;
	}
	if (orientation.translation.y >= maxBorderY)
	{
		orientation.translation.y = maxBorderY;
		direction.y = -direction.y;
	}

}