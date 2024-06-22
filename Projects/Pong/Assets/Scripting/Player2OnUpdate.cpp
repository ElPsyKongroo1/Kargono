void Player2OnUpdate(uint64_t entity, float deltaTime)
{
	uint16_t userSlot = GetActiveSessionSlot();
	if (userSlot == 1)
	{
		SendAllEntityLocation(entity, TransformComponent_GetTranslation(entity));
	}
}
