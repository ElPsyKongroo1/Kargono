void BallOnCreate(uint64_t entity)
{
	CheckHasComponent(entity, "TransformComponent");
	CheckHasComponent(entity, "Rigidbody2DComponent");
	CheckHasComponent(entity, "AudioComponent");
	Math::vec3 translation = TransformComponent_GetTranslation(entity);
	SetEntityFieldByName(entity, "InitialPosition", (void*)&translation);
	Log("We have entered BallOnCreate");
}


