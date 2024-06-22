void Player1OnCreate(uint64_t entity)
{
	CheckHasComponent(entity, "TransformComponent");
	CheckHasComponent(entity, "Rigidbody2DComponent");
	Math::vec3 translation = TransformComponent_GetTranslation(entity);
	SetEntityFieldByName(entity, "InitialPosition", (void*)&translation);
}
