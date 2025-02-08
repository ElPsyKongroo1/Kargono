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
	class EmitterConfigViewportPanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		EmitterConfigViewportPanel();
		~EmitterConfigViewportPanel();

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
		void DrawGridLines();
		void DrawGizmo();
		void DrawParticleSpawningBounds();
		void DrawToolbarOverlay();

		//=========================
		// External Functionality
		//=========================
		void OnOpenEmitterConfig();
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
		int m_GizmoType{ -1 };
		ViewportData m_ViewportData;
	private:
		// Viewport resources
		Ref<Rendering::Framebuffer> m_ViewportFramebuffer;
		bool m_ViewportFocused{ false };
		bool m_ViewportHovered{ false };
		Math::vec2 m_ScreenViewportBounds[2];

		// Debug overlay data
		bool m_ToolbarEnabled{ true };
		bool m_DisplaySpawningBoundsOutline{ true };

		// Draw world axis Data
		float m_LargeGridSpacing{ 100.0f };
		float m_FineGridSpacing{ 5.0f };
		bool m_DisplayXYMajorGrid{ false };
		bool m_DisplayXZMajorGrid{ true };
		bool m_DisplayYZMajorGrid{ false };
		bool m_DisplayXYMinorGrid{ false };
		bool m_DisplayXZMinorGrid{ true };
		bool m_DisplayYZMinorGrid{ false };
	};
}
