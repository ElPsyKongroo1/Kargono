#pragma once

#include "Kargono.h"
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Events/ApplicationEvent.h"
#include "Kargono/Renderer/EditorCamera.h"

#include <filesystem>


namespace Kargono {
	//============================================================
	// Runtime Layer Class
	//============================================================
	// This class holds all of the runtime application logic. The concept of
	//		layers is in its infancy currently. This class holds the logic for
	//		all of the different systems that the runtime uses such as rendering,
	//		audio, and physics.
	class RuntimeLayer : public Layer
	{
	public:
		//==========================
		// Constructor/Destructor
		//==========================
		// Does nothing other than calling parent constructor
		RuntimeLayer();
		virtual ~RuntimeLayer() = default;
		//==========================
		// LifeCycle Functions
		//==========================
		// Starts Audio System and askes the user what project to open(TEMPORARY TODO).
		virtual void OnAttach() override;
		// Closes Audio System
		virtual void OnDetach() override;
		// Calls Runtime Update for Physics, Scripting, and Rendering.
		virtual void OnUpdate(Timestep ts) override;
		// Currently does nothing!
		virtual void OnEvent(Event& event) override;
	private:
		// This function renders the active scene, updates scripts, and updates the scene's physics.
		void OnUpdateRuntime(Timestep ts);
		// This function responds to application collision events. Currently it plays a sound(Very Temporary).
		bool OnPhysicsCollision(PhysicsCollisionEvent event);
		// Logic to open the project and its main scene
		//==========================
		// Project Management
		//==========================
		bool OpenProject();
		void OpenProject(const std::filesystem::path& path);
		void OpenScene();
		void OpenScene(const std::filesystem::path& path);

		// Starts and stops the runtime. This includes Scripting,
		//		Physics, and supporting functionality.
		void OnScenePlay();
		void OnSceneStop();

	private:
		// Current Scene
		Ref<Scene> m_ActiveScene;
		std::filesystem::path m_ScenePath;

		// Viewport Data
		bool m_ViewportFocused = false, m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = {0.0f, 0.0f};
		glm::vec2 m_ViewportBounds[2];

	};

}
