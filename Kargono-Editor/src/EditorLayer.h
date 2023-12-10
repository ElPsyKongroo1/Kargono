#pragma once
#include "Kargono.h"
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Renderer/EditorCamera.h"
#include "Kargono/Text/TextEngine.h"
#include "Kargono/Assets/Asset.h"

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/LogPanel.h"

#include <filesystem>


namespace Kargono {

	//============================================================
	// Editor Layer Class
	//============================================================
	// This class holds all of the editor application logic. The concept of
	//		layers is in its infancy currently. This class holds the logic for
	//		all of the different systems that the editor uses such as scene management,
	//		ImGui code for editor windows, and project management.
	class EditorLayer : public Layer
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		// The constructor simply calls the parent constructor and initializes
		//		the ImGui layer for window use.
		EditorLayer();
		virtual ~EditorLayer() override = default;
		//=========================
		// LifeCycle Functions
		//=========================
		// Think of this functions as a normal Init() function. Currently, this
		//		function initializes the audio system, initializes editor icon
		//		resources, sets up framebuffer for the viewport, initializes
		//		default scene resources, initializes editor boolean values,
		//		requests the user select a project to open, and opens the selected
		//		project. There are other things this function does, take a look at it.
		virtual void OnAttach() override;
		// This function simply closes and cleans up the audio system.
	private:
		// This function initializes the static data used by the OnOverlayRender()
		//		function below. This function is called in OnAttach().
		void InitializeOverlayData();
	public:
		virtual void OnDetach() override;
		// This is a fairly large function and is at the heart of this application.
		//		This function runs every frame. Here is the current functionality:
		//		1. Request viewport resize on Scene to ensure in-runtime cameras are
		//		consistent with the screen aspect ratio
		//		2. Run render commands, physics updates, and script updates depending
		//		on the current state of the editor. The editor can be in the following
		//		states: Edit, Simulate, and Play.
		//		3. Run mouse picking logic to allow the selection of entities with the
		//		mouse in Edit mode.
		//		4. Draw overlay code such as physics colliders, entity selection visualization,
		//		text/ui, etc...
		virtual void OnUpdate(Timestep ts) override;
	private:
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
		// Increments step
		void Step(int frames = 1);
		
	public:
		// This is a large function but its output is fairly simple. The function manages the ImGui
		//		code that displays all of the panels (including the viewport) shown in the editor.
		//		Here is the functionality:
		//		1. The function starts by managing the dockspace that allows panels to be dynamically
		//		moved in the editor.
		//		2. If editor is in the Play or Simulate mode and m_RuntimeFullscreen is true, the editor
		//		simply displays the UI_Viewport() and UI_Toolbar() panels to simulate windowed fullscreen.
		//		3. The following code manages the top-left menu bar adn its options.
		//		4. The final section conditionally displays UI_Panels based on set boolean values(these are
		//		set in the toolbar above).
		virtual void OnImGuiRender() override;
	private:
		// These functions represent UI Panel code that gets optionally called based on boolean values
		//		set in the editor.
		// This toolbar allows the changing of the scene state between Edit, Play, and Simulate.
		//		This toolbar can also pause and step an application that is Playing.
		void UI_Toolbar();
		// This function provides access to various editor settings. Take a look.
		void UI_Settings();
		// This function displays various editor statistics such as drawcall data and whatnot from the renderer.
		void UI_Stats();
		// This is the actual viewport that shows the current Scene world.
		void UI_Viewport();
		// This panel allows the editor to create and manage runtime user interfaces
		void UI_UserInterface();
		// This panel allows the editor to create different input modes that can be swapped out
		void UI_InputEditor();
		// This function exposes project details and allows certain settings to be changed for the currently
		//		loaded project.
		void UI_Project();

	private:
		// Supporting functions for InputEditor. These functions display different sections of the InputEditor
		//		user interface.
		void InputEditor_Keyboard_Events();
		void InputEditor_Keyboard_Polling();

	public:
		// This function catches thrown application events and dispatches them to other functions
		//		(OnKeyPressed(), OnMouseButtonPressed(), and OnPhysicsCollision()).
		//		Those functions proceed to run logic that responds to the thrown event.
		virtual void OnEvent(Event& event) override;
	private:
		// These private functions are called by the above OnEvent(e) function to handle application events.
		// These next two functions provide different code to respond to user input.
		bool OnKeyPressed(KeyPressedEvent event);
		bool OnMouseButtonPressed(MouseButtonPressedEvent event);
		// This function responds to application collision events.
		bool OnPhysicsCollision(PhysicsCollisionEvent event);
	public:
		//=========================
		// Getters/Setters
		//=========================
		static EditorLayer* GetCurrentLayer() { return s_EditorLayer; }
		EditorCamera& GetEditorCamera() { return m_EditorCamera; }
	private:
		//=========================
		// Scene/Project Management
		//=========================
		// These functions are called from different parts of the editor to allow smooth
		//		transitions between the current Project and other projects. Also allows
		//		the current project to save.
		void NewProject();
		bool OpenProject();
		void OpenProject(const std::filesystem::path& path);
		void SaveProject();
		// These functions allow transitioning between scenes in the editor and saving
		//		the current scene.
		void NewScene();
		void OpenScene();
	public:
		void OpenScene(const std::filesystem::path& path);
	private:
		void OpenScene(AssetHandle newScene);
		void SaveScene();
		// This function is called by SaveSceneAs() to actually serialize the scene.
		void SerializeScene(Ref<Scene> scene);

		//=========================
		// Scene State Transitions
		//=========================
		void OnScenePlay();
		void OnSceneSimulate();
		void OnSceneStop();
		void OnScenePause();

		// Extra function. Allows the duplicate of an entity in the current m_EditorScene
		void OnDuplicateEntity();
	private:
		//=========================
		// Private Fields
		//=========================
		static EditorLayer* s_EditorLayer;
		// Booleans to display UI Windows
		bool m_ShowSceneHierarchy = true;
		bool m_ShowContentBrowser = true;
		bool m_ShowLog = false;
		bool m_ShowStats = false;
		bool m_ShowSettings = true;
		bool m_ShowViewport = true;
		bool m_ShowToolbar = true;
		bool m_ShowProject = false;
		bool m_ShowDemoWindow = false;
		bool m_ShowUserInterfaceEditor = false;
		bool m_ShowInputEditor = false;

		// Settings UI Booleans
		bool m_ShowPhysicsColliders = false;
		bool m_ShowCameraFrustrums = true;
		bool m_RuntimeFullscreen = false;
		bool m_ShowUserInterface = true;
		// Editor Scenes
		Ref<Scene> m_ActiveScene;
		Ref<Scene> m_EditorScene;
		AssetHandle m_EditorSceneHandle;

		// Misc
		Entity m_HoveredEntity;
		EditorCamera m_EditorCamera;

		// Viewport Resources
		Ref<Framebuffer> m_ViewportFramebuffer;
		bool m_ViewportFocused = false, m_ViewportHovered = false;
		glm::uvec2 m_ViewportSize = {0, 0};
		glm::vec2 m_ViewportBounds[2];

		// ImGuizmo Resources
		int m_GizmoType = -1;

		// Current Editor State Enum and Field
		enum class SceneState
		{
			Edit = 0, Play = 1, Simulate = 2
		};
		SceneState m_SceneState = SceneState::Edit;
		// Fields allow Stepping
		bool m_IsPaused = false;
		int m_StepFrames = 0;

		// Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		Scope<ContentBrowserPanel>  m_ContentBrowserPanel;
		Scope<LogPanel>  m_LogPanel;
	};

}
