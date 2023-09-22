#pragma once
#include "Kargono.h"
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Renderer/EditorCamera.h"

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
		virtual void OnEvent(Event& event) override;
	private:

		bool OpenProject();
		void OpenProject(const std::filesystem::path& path);

		void OpenScene();
		void OpenScene(const std::filesystem::path& path);

		void OnScenePlay();
		void OnSceneStop();

	private:

		Ref<Scene> m_ActiveScene;
		std::filesystem::path m_ScenePath;

		Entity m_HoveredEntity;

		EditorCamera m_EditorCamera;

		bool m_ViewportFocused = false, m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = {0.0f, 0.0f};
		glm::vec2 m_ViewportBounds[2];

		AudioContext* m_EditorAudio = nullptr;

	};

}
