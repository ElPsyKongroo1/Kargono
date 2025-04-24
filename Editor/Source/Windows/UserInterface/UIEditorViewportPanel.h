#pragma once
#include "EventsPlugin/KeyEvent.h"
#include "Kargono/Core/Timestep.h"
#include "RenderingPlugin/EditorOrthographicCamera.h"
#include "RenderingPlugin/Framebuffer.h"
#include "Kargono/Core/Window.h"
#include "RuntimeUIPlugin/RuntimeUI.h"

#include <string>
#include <limits>

namespace Kargono::Panels
{
	class UIEditorViewportPanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		UIEditorViewportPanel();
		~UIEditorViewportPanel();

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

		void DrawOverlay();
		void DrawUnderlay();
		void DrawViewportOutline();
		void HandleMouseHovering();
		void DrawToolbarOverlay();
		void DrawProportionalGrid();
		void DrawGizmo();

		//=========================
		// External Functionality
		//=========================
		void OnOpenUI();
		Math::vec2 GetMouseViewportPosition();
		ViewportData* GetViewportData();
	private:
		//=========================
		// Supporting Functions
		//=========================
		void DrawDebugLines();
		void DrawWidgetConstraintDistanceLines(RuntimeUI::Widget* widget, const Math::mat4& widgetTransform, const Math::vec3& widgetTranslation);
		void DrawHorizontalContainerElementOutline(RuntimeUI::Widget* widget, const Math::vec3& widgetTranslation, const Math::vec3& widgetSize);
		void DrawVerticalContainerElementOutline(RuntimeUI::Widget* widget, const Math::vec3& widgetTranslation, const Math::vec3& widgetSize);
		void DrawWindowConstraintDistanceLines(RuntimeUI::Window* window);
		void ResetCamera();
		
	public:
		//=========================
		// Core Panel Data
		//=========================
		FixedString32 m_PanelName{ "User Interface Viewport" };
		Rendering::EditorOrthographicCamera m_EditorCamera;
		ViewportData m_ViewportData;
		Math::uvec2 m_ViewportAspectRatio{ 1, 1 };
		int m_GizmoType{ -1 };
	private:
		// Viewport resources
		Ref<Rendering::Framebuffer> m_ViewportFramebuffer;
		bool m_ViewportFocused{ false };
		bool m_ViewportHovered{ false };
		Math::vec2 m_ScreenViewportBounds[2];
		int32_t m_HoveredWindowWidgetID{ Kargono::RuntimeUI::k_InvalidWidgetIndex };

		// Debug overlay data
		bool m_ToolbarEnabled{ true };
		bool m_DisplayGrid{ true };
		float m_PropertionalGridSections{ 2.0f };
	};
}
