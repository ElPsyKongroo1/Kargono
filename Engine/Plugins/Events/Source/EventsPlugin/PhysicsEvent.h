#pragma once

#include "EventsPlugin/Event.h"
#include "Kargono/Core/UUID.h"

#include <sstream>

namespace Kargono::Events
{
	//============================================================
	// Physics Collision Event Class
	//============================================================
	// This event represents a collision that occurs inside the Physics system.
	//		A collision occurs between two entities and the UUID of each entity
	//		is recorded. This information is used later in the event pipeline
	//		to handle concepts such as audio effects.
	class PhysicsCollisionStart : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		// This event is initialized with the UUID of both entities involved
		//		in the collision.
		PhysicsCollisionStart(UUID entityOne, UUID entityTwo)
			: m_EntityOne(entityOne), m_EntityTwo(entityTwo) {}

		//==============================
		// Getters/Setters
		//==============================

		UUID GetEntityOne() const { return m_EntityOne; }
		UUID GetEntityTwo() const { return m_EntityTwo; }

		virtual EventType GetEventType() const override { return EventType::PhysicsCollisionStart; }
		virtual int GetCategoryFlags() const override { return EventCategory::Physics; }
	private:
		// These are the UUID's of both entities involved in the collision event.
		UUID m_EntityOne;
		UUID m_EntityTwo;
	};

	//============================================================
	// Physics Collision End Class
	//============================================================
	class PhysicsCollisionEnd : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		// This event is initialized with the UUID of both entities involved
		//		in the collision.
		PhysicsCollisionEnd(UUID entityOne, UUID entityTwo)
			: m_EntityOne(entityOne), m_EntityTwo(entityTwo) {}

		//==============================
		// Getters/Setters
		//==============================

		UUID GetEntityOne() const { return m_EntityOne; }
		UUID GetEntityTwo() const { return m_EntityTwo; }

		virtual EventType GetEventType() const override { return EventType::PhysicsCollisionEnd; }
		virtual int GetCategoryFlags() const override { return EventCategory::Physics; }
	private:
		// These are the UUID's of both entities involved in the collision event.
		UUID m_EntityOne;
		UUID m_EntityTwo;
	};
}
