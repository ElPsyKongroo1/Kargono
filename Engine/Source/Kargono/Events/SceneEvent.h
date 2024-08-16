#pragma once

#include "Kargono/Events/Event.h"
#include "Kargono/Core/UUID.h"

#include <sstream>

namespace Kargono::Events
{
	//============================================================
	// Delete Entity Class
	//============================================================
	class DeleteEntity : public Event
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		// This event is initialized with the entity ID of the recently deleted entity
		DeleteEntity(UUID id)
			: m_ID(id) {}

		//==============================
		// Getters/Setters
		//==============================

		UUID GetID() const { return m_ID; }

		virtual EventType GetEventType() const override { return EventType::DeleteEntity; }
		virtual int GetCategoryFlags() const override { return EventCategory::Scene; }
	private:
		// ID represents entity that was recently deleted
		UUID m_ID;
	};
}
