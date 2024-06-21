void Player1OnCreate(uint64_t entity)
{
	CheckHasComponent(entity, "TransformComponent");
	CheckHasComponent(entity, "Rigidbody2DComponent");
	//SetEntityField<Math::vec3>(static_cast<Math::vec3>(TransformComponent_GetTranslation(entity)));
}
















