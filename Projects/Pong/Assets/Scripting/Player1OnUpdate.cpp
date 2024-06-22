void Player1OnUpdate(uint64_t entity, float deltaTime)
{
	uint16_t userSlot = GetActiveSessionSlot();
	if (userSlot == 0)
	{
		SendAllEntityLocation(entity, TransformComponent_GetTranslation(entity));
	}
}
