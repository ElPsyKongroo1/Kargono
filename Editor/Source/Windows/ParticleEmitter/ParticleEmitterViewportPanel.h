#if 0

#pragma once
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Core/Timestep.h"
#include "Kargono/Rendering/EditorPerspectiveCamera.h"
#include "Kargono/Rendering/Framebuffer.h"
#include "Kargono/Core/Window.h"

#include <string>
#include <limits>

namespace Kargono::Panels
{
	class ParticleEmitterViewportPanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		ParticleEmitterViewportPanel();
		~ParticleEmitterViewportPanel();

	private:
		//=========================
		// Initialization Functions
		//=========================
		void InitializeFrameBuffer();
		void InitializeOverlayData();

	private:
		//=========================
		// Termination Functions
		//=========================
		void ClearOverlayData();
	public:
		//=========================
		// On Event Functions
		//=========================
		void OnUpdate(Timestep ts);
		void OnEditorUIRender();
		void OnInputEvent(Events::Event* event);
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);

		//=========================
		// Draw overlay
		//=========================
		void HandleMouseHovering();
		void DrawToolbarOverlay();
		void DrawGrid();
		void DrawGizmo();

		//=========================
		// External Functionality
		//=========================
		void OnOpenParticleEmitter();
	private:
		//=========================
		// Supporting Functions
		//=========================
		void ResetCamera();
		
	public:
		//=========================
		// Core Panel Data
		//=========================
		FixedString32 m_PanelName{ "Particle Emitter Viewport" };
		Rendering::EditorPerspectiveCamera m_EditorCamera;
		ViewportData m_ViewportData;
		Math::uvec2 m_ViewportAspectRatio{ 1, 1 };
		int m_GizmoType{ -1 };
	private:
		// Viewport resources
		Ref<Rendering::Framebuffer> m_ViewportFramebuffer;
		bool m_ViewportFocused{ false };
		bool m_ViewportHovered{ false };
		Math::vec2 m_ScreenViewportBounds[2];

		// Debug overlay data
		bool m_ToolbarEnabled{ true };
		bool m_DisplayGrid{ true };
	};
}

#endif
