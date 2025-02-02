#pragma once

#include "Kargono.h"

#include <vector>

namespace Kargono { class EditorApp; }

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
		void ProcessMousePicking();
		// This function runs the overlay code that displays visualization for physics colliders,
		//		entity selection, Text/UI, and Camera Frustrums. This private function is called
		//		in OnUpdate().
		void OnOverlayRender();
		// This function holds the underlying logic to draw Camera Frustrum visualizations.
		//		This function is called in OnOverlayRender().
		void DrawFrustrum(ECS::Entity& entity);
		void DrawWorldAxis();
		void DrawDebugOverlay();

		// These three functions selectively call the scene functions on m_ActiveScene to render the scene,
		//		update physics, and update scripts for m_ActiveScene.
		// These functions are called in OnUpdate() depending on the current scene state (Edit, Runtime, and Simulation)

		// This function simply renders the active scene
		void OnUpdateEditor(Timestep ts, Rendering::EditorCamera& camera);
		// This function renders the active scene, updates scripts, and updates the scene's physics.
		void OnUpdateRuntime(Timestep ts);
		// This functions renders the active scene and updates the scene's physics.
		void OnUpdateSimulation(Timestep ts, Rendering::EditorCamera& camera);

		// This function initializes the static data used by the OnOverlayRender()
		//		function below. This function is called in Init().
		void InitializeOverlayData();


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
		Rendering::EditorCamera m_EditorCamera;
	private:
		FixedString32 m_PanelName{ "Viewport" };
		// Viewport resources
		Ref<Rendering::Framebuffer> m_ViewportFramebuffer;
		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;
		Math::vec2 m_ScreenViewportBounds[2];
		int m_GizmoType = -1;

		// Draw world axis Data
		float m_LargeGridSpacing = 100.0f;
		float m_FineGridSpacing = 10.0f;
		bool m_DisplayXYMajorGrid = false;
		bool m_DisplayXZMajorGrid = true;
		bool m_DisplayYZMajorGrid = false;
		bool m_DisplayXYMinorGrid = false;
		bool m_DisplayXZMinorGrid = true;
		bool m_DisplayYZMinorGrid = false;

		// Draw debug lines data
		std::vector<DebugLine> m_DebugLines;
		std::vector<DebugPoint> m_DebugPoints;

		friend Kargono::EditorApp;
	};
}
