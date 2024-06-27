void Player2OnCreate(uint64_t entity)
{
	CheckHasComponent(entity, "TransformComponent");
	CheckHasComponent(entity, "Rigidbody2DComponent");
	SetEntityFieldByName(entity, "InitialPosition", (void*)&RValueToLValue(TransformComponent_GetTranslation(entity)));
}
