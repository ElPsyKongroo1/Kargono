#pragma once

#include "Kargono/Events/Event.h"
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Core/Timestep.h"
#include "Kargono/Core/FixedString.h"
#include "Kargono/ECS/Entity.h"
#include "Kargono/Rendering/EditorPerspectiveCamera.h"
#include "Kargono/Rendering/Framebuffer.h"
#include "Kargono/Math/Spline.h"
#include "Kargono/RuntimeUI/RuntimeUICommon.h"

#include <vector>

namespace Kargono { class EditorApp; }
namespace Kargono::Panels { class TestingPanel; }

namespace Kargono::Panels
{
	struct DebugLine
	{
		Math::vec3 m_StartPoint;
		Math::vec3 m_EndPoint;
	};

	struct DebugPoint
	{
		Math::vec3 m_Point;
	};

	class ViewportPanel
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		ViewportPanel();
		~ViewportPanel();

		//=========================
		// On Event Functions
		//=========================
		void OnUpdate(Timestep ts);
		void OnEditorUIRender();
		void OnInputEvent(Events::Event* event);
		void OnEditorEvent(Events::Event* event);
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);


		//=========================
		// External API
		//=========================
	public:
		void InitializeFrameBuffer();
		void HandleSceneMouseHovering();
		void HandleUIMouseHovering();
		// This function runs the overlay code that displays visualization for physics colliders,
		//		entity selection, Text/UI, and Camera Frustrums. This private function is called
		//		in OnUpdate().
		void OnOverlayRender();
		// This function holds the underlying logic to draw Camera Frustrum visualizations.
		//		This function is called in OnOverlayRender().
		void DrawFrustrum(ECS::Entity& entity);
		void DrawGridLines();
		void DrawDebugLines();
		void DrawSplines();
		void DrawToolbarOverlay();

		// These three functions selectively call the scene functions on m_ActiveScene to render the scene,
		//		update physics, and update scripts for m_ActiveScene.
		// These functions are called in OnUpdate() depending on the current scene state (Edit, Runtime, and Simulation)

		// This function simply renders the active scene
		void OnUpdateEditor(Timestep ts, Rendering::EditorPerspectiveCamera& camera);
		// This function renders the active scene, updates scripts, and updates the scene's physics.
		void OnUpdateRuntime(Timestep ts);
		// This functions renders the active scene and updates the scene's physics.
		void OnUpdateSimulation(Timestep ts, Rendering::EditorPerspectiveCamera& camera);

		// This function initializes the static data used by the OnOverlayRender()
		//		function below. This function is called in Init().
		void InitializeOverlayData();
		void ClearOverlayData();

		void SetViewportAspectRatio(const Math::uvec2& newAspectRatio);


	private:
		//=========================
		// Internal Functionality
		//=========================
		void AddDebugLine(Math::vec3 startPoint, Math::vec3 endPoint);
		void AddDebugPoint(Math::vec3 startPoint);
		void ClearDebugLines();
		void ClearDebugPoints();

	public:
		//=========================
		// Core Panel Data
		//=========================
		FixedString32 m_PanelName{ "Viewport" };
		Rendering::EditorPerspectiveCamera m_EditorCamera;
		int m_GizmoType{ -1 };
		ViewportData m_ViewportData;
		Math::uvec2 m_ViewportAspectRatio{ 1, 1 };
		uint16_t m_HoveredWindowID{ Kargono::RuntimeUI::k_InvalidWindowID };
		uint16_t m_HoveredWidgetID{ Kargono::RuntimeUI::k_InvalidWidgetID };
	private:
		// Viewport resources
		Ref<Rendering::Framebuffer> m_ViewportFramebuffer;
		bool m_ViewportFocused{ false };
		bool m_ViewportHovered{ false };
		Math::vec2 m_ScreenViewportBounds[2];

		// Overlay Data
		bool m_ToolbarEnabled{ true };

		// Draw world axis Data
		float m_LargeGridSpacing{ 100.0f };
		float m_FineGridSpacing{ 10.0f };
		bool m_DisplayXYMajorGrid{ false };
		bool m_DisplayXZMajorGrid {true};
		bool m_DisplayYZMajorGrid {false};
		bool m_DisplayXYMinorGrid {false};
		bool m_DisplayXZMinorGrid {true};
		bool m_DisplayYZMinorGrid{ false };

		// Draw debug lines data
		std::vector<DebugLine> m_DebugLines;
		std::vector<DebugPoint> m_DebugPoints;
		std::vector<Math::Spline> m_DebugSplines;

		friend Kargono::EditorApp;
		friend class Kargono::Panels::TestingPanel;
	};
}
