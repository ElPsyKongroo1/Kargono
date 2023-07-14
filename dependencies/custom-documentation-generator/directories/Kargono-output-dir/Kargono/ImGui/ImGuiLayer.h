#pragma once

#include "Kargono/Layer.h"

/// @namespace Kargono
namespace Kargono 
{
/// @class ImGuiLayer
/// @brief Class for the ImGui layer in the game engine
/// @details This layer is responsible for rendering ImGui elements in the game window
/// @headerfile Layer.h
	class KG_API ImGuiLayer : public Layer
	{
/// @public
	public:
/// @brief Default constructor for the ImGui layer
/// @details This constructor initializes the ImGuiLayer object and sets the member variables
/// @return None
		ImGuiLayer();
/// @brief Default destructor for the ImGui layer
/// @details This destructor releases the ImGuiLayer object and cleans up any resources
/// @return None
		~ImGuiLayer();


/// @brief Function for attaching the ImGui layer to the game engine
/// @details This function is called when the ImGui layer is attached to the game engine
/// @return None
		void OnAttach();
/// @brief Function for detaching the ImGui layer from the game engine
/// @details This function is called when the ImGui layer is detached from the game engine
/// @return None
		void OnDetach();
/// @brief Function for updating the ImGui layer
/// @details This function is called on each update of the ImGui layer
/// @return None
		void OnUpdate();
/// @brief Function for handling events in the ImGui layer
/// @param event The event to be handled
/// @details This function is called when an event occurs in the ImGui layer
/// @return None
		void OnEvent(Event& event);
/// @private
	private:
/// @brief Member variable for storing the current time in the ImGui layer
/// @details This variable is used for keeping track of the time in the ImGui layer
/// @source Layer.h
		float m_Time = 0.0f;

	};
}