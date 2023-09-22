#pragma once
#include "Kargono.h"
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Renderer/EditorCamera.h"

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/LogPanel.h"

#include <filesystem>


namespace Kargono {

	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& event) override;
	private:
		bool OnKeyPressed(KeyPressedEvent event);
		bool OnMouseButtonPressed(MouseButtonPressedEvent event);
		bool OnPhysicsCollision(PhysicsCollisionEvent event);

		void OnUpdateParticles();

		void OnOverlayRender();

		void NewProject();
		bool OpenProject();
		void OpenProject(const std::filesystem::path& path);
		void SaveProject();

		void NewScene();
		void OpenScene();
		void OpenScene(const std::filesystem::path& path);
		void SaveScene();
		void SaveSceneAs();

		void SerializeScene(Ref<Scene> scene, const std::filesystem::path& path);

		void OnScenePlay();
		void OnSceneSimulate();
		void OnSceneStop();
		void OnScenePause();


		void OnDuplicateEntity();

		// UI Panels
		void UI_Toolbar();
		void UI_Settings();
		void UI_Stats();
		void UI_Viewport();
	private:

		// Booleans to display UI Windows
		bool m_ShowSceneHierarchy = true;
		bool m_ShowContentBrowser = true;
		bool m_ShowLog = false;
		bool m_ShowStats = false;
		bool m_ShowSettings = true;
		bool m_ShowViewport = true;
		bool m_ShowToolbar = true;
		bool m_ShowDemoWindow = false;

		// Settings UI Booleans
		bool m_ShowPhysicsColliders = false;
		bool m_RuntimeFullscreen = false;

		Ref<Framebuffer> m_Framebuffer;

		Ref<Scene> m_ActiveScene;
		Ref<Scene> m_EditorScene;
		std::filesystem::path m_EditorScenePath;

		Entity m_HoveredEntity;

		EditorCamera m_EditorCamera;

		bool m_ViewportFocused = false, m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = {0.0f, 0.0f};
		glm::vec2 m_ViewportBounds[2];


		int m_GizmoType = -1;


		enum class SceneState
		{
			Edit = 0, Play = 1, Simulate = 2
		};

		SceneState m_SceneState = SceneState::Edit;
		AudioContext* m_EditorAudio = nullptr;
		AudioBuffer* m_PopSound = nullptr;
		AudioSource* m_PopSource = nullptr;
		AudioSource* m_LowPopSource = nullptr;

		// Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		Scope<ContentBrowserPanel>  m_ContentBrowserPanel;
		Scope<LogPanel>  m_LogPanel;

		// Editor Resources
		Ref<Texture2D> m_IconPlay, m_IconPause, m_IconStop, m_IconStep, m_IconSimulate;
	};

}
