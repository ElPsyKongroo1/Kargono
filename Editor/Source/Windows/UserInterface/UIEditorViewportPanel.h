#pragma once
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Core/Timestep.h"
#include "Kargono/Rendering/EditorCamera.h"
#include "Kargono/Rendering/Framebuffer.h"
#include "Kargono/Core/Window.h"

#include <string>

namespace Kargono::Panels
{
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
		void ProcessMousePicking();
		
	public:
		//=========================
		// Core Panel Data
		//=========================
		FixedString32 m_PanelName{ "User Interface Viewport" };
		Rendering::EditorCamera m_EditorCamera;
		ViewportData m_ViewportData;
	private:
		// Viewport resources
		Ref<Rendering::Framebuffer> m_ViewportFramebuffer;
		bool m_ViewportFocused{ false };
		bool m_ViewportHovered{ false };
		Math::vec2 m_ScreenViewportBounds[2];
		uint16_t m_HoveredWindowID{ std::numeric_limits<uint16_t>().max() };
		uint16_t m_HoveredWidgetID{ std::numeric_limits<uint16_t>().max() };
	};
}
