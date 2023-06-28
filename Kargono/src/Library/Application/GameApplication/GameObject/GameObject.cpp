#include "GameObject.h"
#include "../../../Library.h"
//void GameObject::setBaseDimensions(glm::vec3 baseDimensions)
//{
//		objectBaseDimensions = baseDimensions;
//		objectCurrentDimensions = glm::vec3(
//			((baseDimensions.x / Resources::currentGame->resourceManager->dimensionConversionFactor) * orientation.scale.x),
//			((baseDimensions.y / Resources::currentGame->resourceManager->dimensionConversionFactor) * orientation.scale.y),
//			((baseDimensions.z / Resources::currentGame->resourceManager->dimensionConversionFactor) * orientation.scale.z));
//	}

void GameParticle::Move()
{
	if (direction.length == 0) { return; }
 	orientation.translation = glm::vec3(orientation.translation.x + (direction.x * currentSpeed * Resources::deltaTime),
		orientation.translation.y + (direction.y * currentSpeed * Resources::deltaTime),
		0.0f);
	

}

