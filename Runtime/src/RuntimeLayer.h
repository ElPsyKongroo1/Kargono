#pragma once

#include "Kargono.h"

#include <filesystem>


namespace Kargono
{
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
		// Closes Audio System and calls OnSceneStop()
		virtual void OnDetach() override;
		// Calls Runtime Update for Physics, Scripting, and Rendering.
		virtual void OnUpdate(Timestep ts) override;
		// Currently does nothing!
		virtual void OnEvent(Events::Event& event) override;
	private:
		// This function renders the active scene, updates scripts, and updates the scene's physics.
		void OnUpdateRuntime(Timestep ts);
		// This function updates the scene cameras when the window is resized
		bool OnWindowResize(Events::WindowResizeEvent event);
		// This function responds to application collision events. Currently it plays a sound(Very Temporary).
		bool OnPhysicsCollision(Events::PhysicsCollisionEvent event);
		// Logic to open the project and its main scene
		//==========================
		// Project Management
		//==========================

		// This function opens a file dialog and calls the other OpenProject(path) function
		//		if the file dialog was successful.
		bool OpenProject();

		// Function opens project described by the path which includes opening the initial scene
		//		and loading the new app assembly into the scripting engine.
		void OpenProject(const std::filesystem::path& path);

		// This function opens a scene. This function is only currently called by the OpenProject function
		//		to open the initial scene.
		void OpenScene(Assets::AssetHandle sceneHandle);

		// Starts and stops the runtime. This includes Scripting,
		//		Physics, and supporting functionality.
		void OnScenePlay();
		void OnSceneStop();

	};

}
