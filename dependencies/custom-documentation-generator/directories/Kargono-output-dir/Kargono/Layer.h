#pragma once

#include "Kargono/Core.h"
#include "Kargono/Events/Event.h"

namespace Kargono 
{
/// @class Layer
	class KG_API Layer
	{
	public:
/// @brief Constructs a layer object
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnEvent(Event& event) {}

		inline const std::string& GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;

	};
}