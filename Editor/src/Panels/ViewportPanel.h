#pragma once

#include "Kargono.h"

namespace Kargono
{
	class EditorLayer;

	class ViewportPanel
	{
	public:
		ViewportPanel() = default;

		void OnUpdate(Timestep ts);

		void OnEditorUIRender();

		void InitializeFrameBuffer();


		void ProcessMousePicking();
		// This function runs the overlay code that displays visualization for physics colliders,
		//		entity selection, Text/UI, and Camera Frustrums. This private function is called
		//		in OnUpdate().
		void OnOverlayRender();
		// This function holds the underlying logic to draw Camera Frustrum visualizations.
		//		This function is called in OnOverlayRender().
		void DrawFrustrum(Entity& entity);

		// These three functions selectively call the scene functions on m_ActiveScene to render the scene,
		//		update physics, and update scripts for m_ActiveScene.
		// These functions are called in OnUpdate() depending on the current scene state (Edit, Runtime, and Simulation)

		// This function simply renders the active scene
		void OnUpdateEditor(Timestep ts, EditorCamera& camera);
		// This function renders the active scene, updates scripts, and updates the scene's physics.
		void OnUpdateRuntime(Timestep ts);
		// This functions renders the active scene and updates the scene's physics.
		void OnUpdateSimulation(Timestep ts, EditorCamera& camera);

		// This function initializes the static data used by the OnOverlayRender()
		//		function below. This function is called in OnAttach().
		void InitializeOverlayData();
	public:
		EditorCamera m_EditorCamera;
	private:
		// Viewport Resources
		Ref<Framebuffer> m_ViewportFramebuffer;
		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;
		Math::vec2 m_ViewportBounds[2];
		int m_GizmoType = -1;

		friend EditorLayer;
	};
}
