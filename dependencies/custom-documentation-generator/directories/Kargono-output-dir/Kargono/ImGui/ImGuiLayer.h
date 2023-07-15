
#pragma once

/// @brief Include the Layer header file
#include "Kargono/Layer.h"
/// @brief Include the ApplicationEvent header file
#include "Kargono/Events/ApplicationEvent.h"
/// @brief Include the KeyEvent header file
#include "Kargono/Events/KeyEvent.h"
/// @brief Include the MouseEvent header file
#include "Kargono/Events/MouseEvent.h"



namespace Kargono 
{
/// @class ImGuiLayer

/// @brief Represents an ImGui layer that inherits from the Layer class.

/// @details This layer is responsible for rendering ImGui elements.
	class KG_API ImGuiLayer : public Layer
	{

	public:
/// @brief Default constructor for the ImGuiLayer class.
		ImGuiLayer();
/// @brief Destructor for the ImGuiLayer class.
		~ImGuiLayer();


/// @brief Function that is called when the layer is attached to the application.
		virtual void OnAttach() override;
/// @brief Function that is called when the layer is detached from the application.
		virtual void OnDetach() override;
/// @brief Function that is called to render ImGui elements.
		virtual void OnImGuiRender() override;

/// @brief Function that begins ImGui rendering.
		void Begin();
/// @brief Function that ends ImGui rendering.
		void End();
		



	private:
/// @brief The current time for ImGui rendering.
		float m_Time = 0.0f;

	};
}