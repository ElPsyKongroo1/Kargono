#include "BreakoutObject.h"
#include "../../../Library/Library.h"
void GameBall::Move()
{
	orientation.translation = glm::vec3(orientation.translation.x + (direction.x * currentSpeed * Resources::deltaTime),
		orientation.translation.y + (direction.y * currentSpeed * Resources::deltaTime),
		0.0f);
	float maxBorderX = Resources::currentGame->renderer->currentCamera->frustrumDimensions.widthDimension.y - (static_cast<ShapeRenderer*>(renderer)->mesh->dimensions.x * orientation.scale.x);
	float minBorderX = -(Resources::currentGame->renderer->currentCamera->frustrumDimensions.widthDimension.y - (static_cast<ShapeRenderer*>(renderer)->mesh->dimensions.x * orientation.scale.x));
	float minBorderY = -(Resources::currentGame->renderer->currentCamera->frustrumDimensions.heightDimension.y - (static_cast<ShapeRenderer*>(renderer)->mesh->dimensions.y * orientation.scale.y));
	float maxBorderY = Resources::currentGame->renderer->currentCamera->frustrumDimensions.heightDimension.y - (static_cast<ShapeRenderer*>(renderer)->mesh->dimensions.y * orientation.scale.y);
	if (orientation.translation.x >= maxBorderX)
	{
		orientation.translation.x = maxBorderX;
		direction.x = -direction.x;
		direction = glm::normalize(glm::vec3(direction.x, direction.y * 1.25f, 0.0f));
	}
	if (orientation.translation.x <= minBorderX)
	{
		orientation.translation.x = minBorderX;
		direction.x = -direction.x;
		direction = glm::normalize(glm::vec3(direction.x, direction.y * 1.25f, 0.0f));
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