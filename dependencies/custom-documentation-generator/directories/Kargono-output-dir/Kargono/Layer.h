#pragma once

#include "Kargono/Core.h"
#include "Kargono/Events/Event.h"

namespace Kargono 
{
/// @class Layer
/// @brief Base class for all layers in the game engine
	class KG_API Layer
	{
/// @brief Public member functions of the layer class
	public:
/// @brief Constructor for the Layer class
/// @param name The name of the layer
		Layer(const std::string& name = "Layer");
/// @brief Virtual destructor for the Layer class
		virtual ~Layer();

/// @brief Called when the layer is attached to the application
		virtual void OnAttach() {}
/// @brief Called when the layer is detached from the application
		virtual void OnDetach() {}
/// @brief Called to update the layer
		virtual void OnUpdate() {}
/// @brief Called to render the ImGui for the layer
		virtual void OnImGuiRender() {}
/// @brief Called when an event occurs in the layer
/// @param event The event that occurred
		virtual void OnEvent(Event& event) {}

/// @brief Returns the name of the layer
/// @return The name of the layer
		inline const std::string& GetName() const { return m_DebugName; }
/// @brief Protected member functions of the layer class
	protected:
/// @brief The debug name of the layer
		std::string m_DebugName;

	};
}