void BallOnCreate(uint64_t entity)
{
	CheckHasComponent(entity, "TransformComponent");
	CheckHasComponent(entity, "Rigidbody2DComponent");
	CheckHasComponent(entity, "AudioComponent");
	SetEntityFieldByName(entity, "InitialPosition", (void*)&RValueToLValue(TransformComponent_GetTranslation(entity)));
}
