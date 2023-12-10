#pragma once

#include "Kargono.h"
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Events/ApplicationEvent.h"
#include "Kargono/Renderer/EditorCamera.h"
#include "Kargono/Assets/Asset.h"

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
		// This function updates the scene cameras when the window is resized
		bool OnWindowResize(WindowResizeEvent event);
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
		void OpenScene(AssetHandle sceneHandle);

		// Starts and stops the runtime. This includes Scripting,
		//		Physics, and supporting functionality.
		void OnScenePlay();
		void OnSceneStop();

	private:
		// Current Scene
		Ref<Scene> m_ActiveScene;
		AssetHandle m_SceneHandle;

	};

}
