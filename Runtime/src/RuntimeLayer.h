#pragma once
#include "Kargono.h"
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Renderer/EditorCamera.h"

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/LogPanel.h"

#include <filesystem>


namespace Kargono {

	class RuntimeLayer : public Layer
	{
	public:
		RuntimeLayer();
		virtual ~RuntimeLayer() = default;
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& event) override;
	private:
		void OnOverlayRender();

		void NewProject();
		bool OpenProject();
		void OpenProject(const std::filesystem::path& path);

		void NewScene();
		void OpenScene();
		void OpenScene(const std::filesystem::path& path);

		void OnScenePlay();
		void OnSceneStop();

		// UI Panels
		void UI_Viewport();
	private:

		Ref<Framebuffer> m_Framebuffer;

		Ref<Scene> m_ActiveScene;
		std::filesystem::path m_ScenePath;

		Entity m_HoveredEntity;

		EditorCamera m_EditorCamera;

		bool m_ViewportFocused = false, m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = {0.0f, 0.0f};
		glm::vec2 m_ViewportBounds[2];


		int m_GizmoType = -1;

		AudioContext* m_EditorAudio = nullptr;

		// Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		Scope<ContentBrowserPanel>  m_ContentBrowserPanel;
		Scope<LogPanel>  m_LogPanel;

		// Editor Resources
		Ref<Texture2D> m_IconPlay, m_IconPause, m_IconStop, m_IconStep, m_IconSimulate;
	};

}
