bool BallOnPhysicsCollision(uint64_t entity,uint64_t otherEntity)
{
	uint16_t userSlot = GetActiveSessionSlot();
	if (userSlot == std::numeric_limits<uint16_t>().max())
	{
		return BallOfflineCollision(entity, otherEntity);
	}
	else
	{
		return BallOnlineCollision(entity, userSlot, otherEntity);
	}
}







