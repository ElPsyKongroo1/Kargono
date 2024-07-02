#pragma once

#include "Kargono.h"

namespace Kargono { class EditorApp; }

namespace Kargono::Panels
{
	class ViewportPanel
	{
	public:
		ViewportPanel();

		void OnUpdate(Timestep ts);

		void OnEditorUIRender();
		void OnEvent(Events::Event& e);
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);

		void InitializeFrameBuffer();

		void ProcessMousePicking();
		// This function runs the overlay code that displays visualization for physics colliders,
		//		entity selection, Text/UI, and Camera Frustrums. This private function is called
		//		in OnUpdate().
		void OnOverlayRender();
		// This function holds the underlying logic to draw Camera Frustrum visualizations.
		//		This function is called in OnOverlayRender().
		void DrawFrustrum(Scenes::Entity& entity);

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
		//		function below. This function is called in OnAttach().
		void InitializeOverlayData();
	public:
		Rendering::EditorCamera m_EditorCamera;
	private:
		std::string m_PanelName{ "Viewport" };
		// Viewport Resources
		Ref<Rendering::Framebuffer> m_ViewportFramebuffer;
		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;
		Math::vec2 m_ViewportBounds[2];
		int m_GizmoType = -1;

		friend Kargono::EditorApp;
	};
}
