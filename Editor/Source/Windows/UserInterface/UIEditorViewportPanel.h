#pragma once
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Core/Timestep.h"
#include "Kargono/Rendering/EditorOrthographicCamera.h"
#include "Kargono/Rendering/Framebuffer.h"
#include "Kargono/Core/Window.h"
#include "Kargono/RuntimeUI/RuntimeUI.h"

#include <string>
#include <limits>

namespace Kargono::Panels
{
	constexpr uint16_t k_InvalidWindowID{ std::numeric_limits<uint16_t>().max() };
	constexpr uint16_t k_InvalidWidgetID{ std::numeric_limits<uint16_t>().max() };

	class UIEditorViewportPanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		UIEditorViewportPanel();

	private:
		//=========================
		// Initialization Functions
		//=========================
		void InitializeFrameBuffer();
		void InitializeOverlayData();

	public:
		//=========================
		// On Event Functions
		//=========================
		void OnUpdate(Timestep ts);
		void OnEditorUIRender();

		void DrawOverlay();
		void HandleMouseHovering();
		
	private:
		//=========================
		// Supporting Functions
		//=========================
		void DrawWindowWidgetDebugLines();
		void DrawWidgetConstraintDistanceLines(RuntimeUI::Window* window, RuntimeUI::Widget* widget, const Math::mat4& widgetTransform, const Math::vec3& widgetTranslation);
		void DrawWindowConstraintDistanceLines(RuntimeUI::Window* window);
		
	public:
		//=========================
		// Core Panel Data
		//=========================
		FixedString32 m_PanelName{ "User Interface Viewport" };
		Rendering::EditorOrthographicCamera m_EditorCamera;
		ViewportData m_ViewportData;
	private:
		// Viewport resources
		Ref<Rendering::Framebuffer> m_ViewportFramebuffer;
		bool m_ViewportFocused{ false };
		bool m_ViewportHovered{ false };
		Math::vec2 m_ScreenViewportBounds[2];
		uint16_t m_HoveredWindowID{ k_InvalidWindowID };
		uint16_t m_HoveredWidgetID{ k_InvalidWidgetID };
	};
}
