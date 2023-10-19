#include "kgpch.h"

#include "Kargono/Physics/Physics2D.h"
#include "Kargono/Core/Application.h"

#include "box2d/b2_contact.h"

namespace Kargono
{
	ContactListener::ContactListener()
		: b2ContactListener()
	{
	}
	void ContactListener::BeginContact(b2Contact* contact)
	{
		KG_CORE_ASSERT(m_CallbackFunc, "Missing Callback Function to link to Event Dispatch!");
		UUID entityOne = contact->GetFixtureA()->GetBody()->GetUserData().UUID;
		UUID entityTwo = contact->GetFixtureB()->GetBody()->GetUserData().UUID;
		PhysicsCollisionEvent event = PhysicsCollisionEvent(entityOne, entityTwo);
		m_CallbackFunc(event);
	}

	
}
